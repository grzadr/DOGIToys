#include "dogitoys.h"

#include <QDebug>
#include <QMap>
#include <unordered_set>

using namespace DOGIToys;

using namespace DOGITools;
using DOGIGFF::GFFRegulation;
using DOGIGFF::GVFStructural;

void DOGI::insertMapMGI(int id_mgi, const QString &id_database,
                        int id_feature) {
  QSqlQuery query(*db);

  prepare(query, "INSERT INTO MapMGI (id_mgi, id_database, id_feature) "
                 "VALUES(:id_mgi, :id_database, :id_feature)");

  query.bindValue(":id_mgi", id_mgi);
  query.bindValue(":id_database", id_database);
  query.bindValue(":id_feature", id_feature);

  exec(query);
}

void DOGI::populateVarStructural(const QVector<GFFFile> &gvf) {

  logger->info("[Populate] [VarStructural] Engaged ({} files)", gvf.size());

  for (const auto &[id_database, file_name] : gvf)
    populateVarStructural(id_database, file_name);

  logger->info("[Populate] [VarStructural] Completed");
}

void DOGI::populateVarStructural(const QString &id_database,
                                 const QString &file_name) {

  struct GVFFeature {
    IDFeature child;
    QString child_signature;
    QPair<QString, QString> parent_signature;
  };

  transaction();

  initVariants();

  logger->info("[Populate] [VarStructural] {}", file_name.toStdString());

  exec("DELETE FROM GVFFEatures WHERE id_database = '" + id_database + "'");

  DOGIFile file(file_name);

  QString line{};
  file.skipLines('#', &line);

  QVector<GVFFeature> children;
  QHash<QPair<QString, QString>, IDFeature> parents;

  int id_feature = DOGIFetch::getMaxIDFeature(*db, "GVFFeatures", id_database);
  size_t line_num = 0;

  logger->info("[Populate] [VarStructural] [Progress] Line {}", line_num);

  do {
    if (++line_num % 50000 == 0)
      logger->info("[Populate] [VarStructural] [Progress] Line {}", line_num);

    if (line.startsWith("#"))
      continue;
    else {
      GVFStructural record{line};
      record.insert(*db, id_database, ++id_feature);
      if (record.hasParent())
        children.append({{id_database, id_feature},
                         record.getSignature(),
                         {record.getSeqID(), record.getParent()}});
      else
        parents.insert({record.getSeqID(), record.getSignature()},
                       {id_database, id_feature});
    }
  } while (file.readLineInto(&line));

  logger->info("[Populate] [VarStructural] Updating Parents");

  for (const auto &[child, signature, parent] : children) {
    if (parents.contains(parent))
      GVFStructural::updateParent(*db, parents.value(parent), child);
    //    else {
    //      cout << "Non-Standard" << endl;
    //      for (const auto &parent_key : parents.keys()) {
    //        if (parent_key.second == parent.second)
    //          GVFRecord::updateParent(*db, parents.value(parent_key), child);
    //      }
    //    }
  }

  logger->info("[Populate] [VarStructural] Commiting");

  commit();

  logger->info("[Populate] [VarStructural] Completed");
}

void DOGI::populateRegulatory(const QVector<GFFFile> &gff) {
  logger->info("[Populate] [Regulatory] Engaged");
  for (const auto &[id_database, file_name] : gff)
    populateRegulatory(id_database, file_name);
  logger->info("[Populate] [Regulatory] Completed");
}

void DOGI::populateRegulatory(const QString &id_database,
                              const QString &file_name) {
  transaction();

  logger->info("[Populate] [Regulatory] {}", file_name.toStdString());

  DOGIFetch::deleteFeatures(*db, "RegulatoryFeatures", id_database);

  DOGIFile file(file_name);

  QString line{};
  file.skipLines('#', &line);

  int id_feature =
      DOGIFetch::getMaxIDFeature(*db, "RegulatoryFeatures", id_database);
  size_t line_num = 0;

  logger->info("[Populate] [Regulatory] [Progress] Line {}", line_num);

  do {
    if (++line_num % 50000 == 0)
      logger->info("[Populate] [Regulatory] [Progress] Line {}", line_num);

    if (line.startsWith("#"))
      continue;
    else {
      GFFRegulation record{line};
      record.insert(*db, id_database, ++id_feature);
    }
  } while (file.readLineInto(&line));

  exec(sqlite_closing);

  commit();

  logger->info("[Populate] [Regulatory] Completed");
}

void DOGI::populate_hpa_tissue_expression(const QString &expression) {

  logger->info("[Populate] [Expression] Engaged");

  transaction();

  DOGIFile file(expression);

  QString id_database{}, old_gene_id{};
  int id_feature{0}, line_num{1};

  QStringList missing{};

  QString line{};

  while (file.readLineInto(&line)) {
    if (++line_num % 100000 == 0) {
      logger->info("[Populate] [Expression] Processed {}", line_num);
    }
    auto data = line.split("\t");

    const auto &gene_id = data[0];
    const auto &id_tissue = data[2].replace(' ', '_');
    //    const auto &gene_name = data[1];
    const auto &tpm = data[3].toDouble();

    QSqlQuery query(*db);

    if (missing.contains(gene_id))
      continue;

    if (old_gene_id != gene_id) {

      prepare(query, "SELECT id_database, id_feature FROM GFF3Features "
                     "WHERE id_database = :id_database AND "
                     "feature_signature = :gene_id");

      query.bindValue(":id_database", "ensembl");
      query.bindValue(":gene_id", gene_id);

      exec(query);

      if (!query.next()) {
        cout << "Not found " << gene_id.toStdString() << endl;
        missing.append(gene_id);
        continue;
        //        throw_error(gene_id + " " + id_tissue);
      }

      id_database = query.value(0).toString();
      id_feature = query.value(1).toInt();
      old_gene_id = gene_id;
    }

    if (query.next())
      throw_error("Multiple genes " + gene_id + " " + id_tissue);

    query.finish();
    prepare(query, "INSERT OR IGNORE INTO HPATissues (id_tissue) "
                   "VALUES (:id_tissue)");
    query.bindValue(":id_tissue", id_tissue);
    exec(query);
    query.finish();

    prepare(query, "INSERT INTO HPAGenesExpression "
                   "(id_database, id_feature, id_tissue, expression_tpm) "
                   "VALUES (:id_database, :id_feature, :id_tissue, :tpm)");

    query.bindValue(":id_database", id_database);
    query.bindValue(":id_feature", id_feature);
    query.bindValue(":id_tissue", id_tissue);
    query.bindValue(":tpm", tpm);

    exec(query);
    query.finish();
  }

  commit();

  logger->info("[Populate] [Expression] Completed");
}
