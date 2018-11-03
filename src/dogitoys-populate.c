#include "dogitoys.h"

#include <QDebug>
#include <QMap>
#include <unordered_set>

using namespace DOGIToys;

using namespace DOGITools;
using DOGIGFF::GFFRegulation;
using DOGIGFF::GVFStructural;

void DOGI::register_annotation(const QString &source, const QString &data) {
  QSqlQuery query(*this->db);
  prepare(query, "INSERT INTO DOGIAnnotations (source, data) "
                 "VALUES (:source, :data)");
  query.bindValue(":source", source);
  query.bindValue(":data", data);
  exec(query);
}

void DOGI::register_table(const QString &name) {
  QSqlQuery query(*this->db);
  prepare(query, "INSERT INTO DOGIMaster (id_field, value) "
                 "VALUES (:name, 'registered')");
  query.bindValue(":name", name);
  exec(query);
}

void DOGI::register_table(const QStringList &names) {
  for (const auto &name : names)
    register_table(name);
}

void DOGI::populate_annotations(const QVector<GFFFile> &files) {
  logger->info("[Populate] [Annotations] Engaged");

  transaction();

  for (const auto &[gff3_source, gff3_path] : files) {
    logger->info("[Populate] [Annotations]");

    QFile file(gff3_path);

    cout << "File: " << file.fileName().toStdString() << endl;
    cout << "Source: " << gff3_source.toStdString() << endl;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      throw_error("Can't open file: " + gff3_path);

    cout << "Organism: " << taxon_id << "(" << taxon_name.toStdString() << ")"
         << endl;

    if (gff3_source != "ensembl" && gff3_source != "ncbi" &&
        gff3_source != "gencode")
      throw_error("Source not supported: " + gff3_source);

    QTextStream in(&file);
    QString line{};
    QHash<QString, int> parents{};

    int id_feature = 1;
    size_t line_num = 0;

    logger->info("[Populate] [Annotations] [Progress] Line {}", line_num);

    while (in.readLineInto(&line)) {
      if (++line_num % 50000 == 0)
        logger->info("[Populate] [Annotations] [Progress] Line {}", line_num);

      if (line.startsWith("#")) {
        process_GFF3Comment(line, gff3_source);
        continue;
      } else {
        auto data = line.split("\t");

        const auto seqid{gff3_str_clean(data[0])};
        const auto source{gff3_str_clean(data[1])};
        const auto type{data[2]};
        const auto first{data[3].toInt()};
        const auto last{data[4].toInt()};
        const auto score{data[5]};
        const auto strand{data[6]};
        const auto phase{data[7]};

        insert_GFF3Feature(gff3_source, id_feature, seqid, source, type, first,
                           last, score, strand, phase);

        try {

          process_GFF3FeatureAttributes(gff3_source, id_feature, type, parents,
                                        data[8]);
        } catch (const runtime_error &ex) {
          cout << line_num << endl;
          cout << ex.what() << endl;
          throw ex;
        }

        ++id_feature;
      }
    }

    register_annotation(gff3_source, gff3_path);

    file.close();
  }

  commit();

  logger->info("[Populate] [Annotations] Completed");
}

void DOGI::populate_ncbi2ensembl(QFile &file) {
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    throw_error("Can't open taxon file: " + file.fileName());
  QTextStream in(&file);
  QString line{};
  //  QSet<int> discarded_taxons, accepted_taxons;

  //  const auto &features_ids = generate_GFF3IDFeatureMap();

  logger->info("[Populate] [NCBI2Ensembl] Collecting names from file");

  auto line_num{0};

  while (in.readLineInto(&line)) {
    if (++line_num % 100000 == 0)
      logger->info("[Populate] [NCBI2Ensembl] [Progress] Line {}", line_num);
    //    logger->info("[Populate] [NCBI2Ensembl] [Progress] Line {}",
    //    line_num);

    if (line.startsWith('#'))
      continue;

    auto data = line.split("\t");

    QSqlQuery taxon_query(*db);

    const auto &taxon = data[0].toInt();

    if (taxon != taxon_id)
      continue;

    //    if (discarded_taxons.contains(taxon))
    //      continue;

    //    if (!accepted_taxons.contains(taxon)) {
    //      prepare(taxon_query, "SELECT 1 FROM DOGIAnnotations "
    //                           "WHERE source IN ('ncbi', 'ensembl') AND "
    //                           "organism = :taxon "
    //                           "GROUP BY organism HAVING COUNT(1) == 2");
    //      taxon_query.bindValue(":taxon", taxon);
    //      exec(taxon_query);

    //      if (!taxon_query.next()) {
    //        discarded_taxons.insert(taxon);
    //        continue;
    //      }

    //      logger->info("Accepted {}", taxon);
    //      accepted_taxons.insert(taxon);
    //    }

    insert_GFF3Mapper("ncbi", data[1], data[3], data[5], "ensembl", data[2],
                      data[4], data[6]);
  }

  logger->info("[Populate] [NCBI2Ensembl] Completed");
}

void DOGI::populateMapperNCBI2Ensembl(const QString &file_name) {
  DOGIFile file(file_name);

  logger->info("[Populate] [NCBI2Ensembl] Engaged");

  exec("DELETE FROM MapGFF3 "
       "WHERE id_database_from = 'ncbi' AND id_database_to = 'ensembl'");

  logger->info("[Populate] [NCBI2Ensembl] Collecting names from file");

  size_t line_num = 0;

  while (file.readLineInto()) {
    if (++line_num % 50000 == 0)
      logger->info("[Populate] [NCBI2Ensembl] [Progress] Line {}", line_num);

    if (file.line.startsWith('#'))
      continue;

    auto data = file.line.split("\t");

    if (const auto &taxon = data[0].toInt(); taxon != taxon_id)
      continue;
    auto ncbi = DOGIFetch::getIDsFromSignature(*db, "ncbi", data[3],
                                               "GFF3Features", true);
    auto ensembl = DOGIFetch::getIDsFromSignature(*db, "ensembl", data[4],
                                                  "GFF3Features", true);
    bool found = ensembl.size() && ncbi.size();

    if (!found) {
      if (data[3] == "-" && data[4] == "-") {
        ncbi = DOGIFetch::getIDsFromSignature(*db, "ncbi", data[1],
                                              "GFF3Features", true);
        ensembl = DOGIFetch::getIDsFromSignature(*db, "ensembl", data[2],
                                                 "GFF3Features", true);
        if (ensembl.isEmpty() or ncbi.isEmpty())
          cout << "Empty genes in line " << line_num << "\n"
               << data[1].toStdString() << " " << data[2].toStdString() << endl;
      } else
        throw_error("Names not found " + QString::number(line_num) + "\n" +
                    data[3] + " " + data[4]);
    }

    for (const auto &[id_database_from, id_feature_from] : ncbi) {
      for (const auto &[id_database_to, id_feature_to] : ensembl)
        DOGIFetch::insertMapGFF3(*db, id_database_from, id_feature_from,
                                 id_database_to, id_feature_to, found, true);
    }
  }

  logger->info("[Populate] [NCBI2Ensembl] Completed");
}

void DOGI::populate_mapper_ensembl_uniprot(const QString &file_name) {
  DOGIFile file(file_name);

  exec("DELETE FROM MapUniprot");

  QString id_database{"ensembl"};
  //  int id_feature{0};

  QSet<QString> added{};

  file.readLineInto();
  while (file.readLineInto()) {
    const auto &data = file.line.split("\t");

    //    if (data[4] != "Uniprot/SWISSPROT")
    //      continue;

    const auto &ensembl_gene_id = data[0];
    const auto &ensembl_transcript_id = data[1];

    const auto &id_uniprot = data[3];
    const auto &uniprot_source = data[4];
    const auto &uniprot_type = data[5];

    if (!added.contains(ensembl_gene_id)) {
      added.insert(ensembl_gene_id);
      const auto &id = getIDFromSignature(id_database, ensembl_gene_id);

      if (id)
        insertMapUniprot((*id).first, (*id).second, id_uniprot, uniprot_source,
                         uniprot_type);
      else
        continue;
    }

    if (const auto id =
            getIDFromSignature(id_database, ensembl_transcript_id)) {
      insertMapUniprot((*id).first, (*id).second, id_uniprot, uniprot_source,
                       uniprot_type);
    }
  }
}

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

void DOGI::populateMapperMGI(const QString &file_name) {
  DOGIFile file(file_name);

  exec("DELETE FROM MapMGI");

  QString ensembl_id_database{"ensembl"}, ncbi_id_database{"ncbi"};

  file.passLines(1);

  while (file.readLineInto()) {
    const auto &data = file.line.split("\t");

    const auto &id_mgi = data[0].mid(data[0].indexOf(':') + 1).toInt();

    if (const auto &ncbi_signature = data[5]; ncbi_signature != "null") {
      if (const auto &features =
              getIDsFromSignature(ncbi_id_database, ncbi_signature);
          features.size())
        for (const auto &[id_database, id_feature] : features)
          insertMapMGI(id_mgi, id_database, id_feature);
      else
        throw_error("Signature not found " + ncbi_signature);
    };

    if (const auto &ensembl_signature = data[10]; ensembl_signature != "null") {
      if (const auto &feature =
              getIDFromSignature(ensembl_id_database, ensembl_signature))
        insertMapMGI(id_mgi, ensembl_id_database, (*feature).second);
      else
        throw_error("Signature not found " + ensembl_signature);
    };
  }
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

void DOGI::insert_chromosome(const QString &id_database, const QString &masking,
                             const QString &id_chrom, const QString &seq) {
  QSqlQuery query(*db);

  prepare(query, "INSERT INTO Genomes (id_database, masking, id_chrom, seq) "
                 "VALUES (:id_database, :masking, :id_chrom, :seq)");

  query.bindValue(":id_database", id_database);
  query.bindValue(":masking", masking);
  query.bindValue(":id_chrom", id_chrom);
  query.bindValue(":seq", seq);

  exec(query);
}

void DOGI::populate_genomes(const QString &id_database, const QString &masking,
                            const QString &file_name) {

  transaction();

  exec("DELETE FROM Genomes WHERE id_database = '" + id_database +
       "' AND masking = '" + masking + "'");

  logger->info("[Populate] [Genomes] {} {}", id_database.toStdString(),
               masking.toStdString());
  FASTAReader file(file_name.toStdString());
  string old_name, name, seq;

  while (file.readFASTASeq(name, seq)) {
    const auto &first_space = name.find(' ');
    if (const auto last_space = name.find_last_of(' ');
        name.substr(last_space + 1) != "REF")
      continue;
    if (const auto type = name.substr(name.find(':', first_space + 1) + 1, 10);
        type != "chromosome")
      continue;
    logger->info("[Populate] [Genomes] [Adding] {}", name);
    insert_chromosome(id_database, masking,
                      QString::fromStdString(name.substr(0, first_space)),
                      QString::fromStdString(seq));
  }

  commit();
}

void DOGI::populate_genomes(const QVector<Genome> &genomes) {
  logger->info("[Populate] [Genomes] Engaged");
  for (const auto &[id_database, masking, file_name] : genomes)
    populate_genomes(id_database, masking, file_name);
  logger->info("[Populate] [Genomes] Completed");
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

void DOGI::insertMapUniprot(const QString &id_database, int id_feature,
                            const QString &id_uniprot,
                            const QString &uniprot_source,
                            const QString &uniprot_type) {

  QSqlQuery query(*db);

  prepare(query,
          "INSERT INTO MapUniprot "
          "(id_database, id_feature, id_uniprot, uniprot_source, uniprot_type) "
          "VALUES(:id_database, :id_feature, :id_uniprot, :uniprot_source, "
          ":uniprot_type)");

  query.bindValue(":id_database", id_database);
  query.bindValue(":id_feature", id_feature);
  query.bindValue(":id_uniprot", id_uniprot);
  query.bindValue(":uniprot_source", uniprot_source);
  query.bindValue(":uniprot_type", uniprot_type);

  //  qDebug() << id_database << id_feature << id_uniprot << uniprot_source
  //           << uniprot_type;

  exec(query);
}

void DOGI::populate_mapper(QVector<Mapper> &mappers) {
  for (const auto &[from, to, path] : mappers)
    populate_mapper(from, to, path);
}

void DOGI::populate_mapper(const QString &database_from,
                           const QString &database_to,
                           const QString &file_name) {

  transaction();
  if (database_from == "uniprot" && database_to == "ensembl")
    populate_mapper_ensembl_uniprot(file_name);
  else if (database_from == "ncbi" && database_to == "ensembl")
    populateMapperNCBI2Ensembl(file_name);
  else if (database_from == "mgi" && database_to == "all")
    populateMapperMGI(file_name);
  else
    throw_error("Unsupported mapping " + database_from + " -> " + database_to);

  commit();
}

void DOGI::populate_go_terms(const QString &file_name, bool init) {

  logger->info(" [Populate] [GOTerms] Engaged");

  transaction();

  logger->info(" [Populate] [GOTerms] Cleaning");

  if (init) {
    logger->info(" [Populate] [GOTerms] Init");
    initGeneOntology();
  }

  exec("DELETE FROM GOHierarchy");
  exec("DELETE FROM GOTerms");

  logger->info(" [Populate] [GOTerms] Parsing file {}",
               file_name.toStdString());

  QVector<QPair<int, int>> hierarchy{};

  DOGIGOParser parser(file_name);
  for (const auto &ele : parser.parse())
    hierarchy.append(DOGIGO::insertGOTerm(*db, ele));

  for (const auto &[id_go, go_is_a] : hierarchy)
    DOGIGO::insertGOHierarchy(*db, id_go, go_is_a);

  commit();

  logger->info(" [Populate] [GOTerms] Completed");
}

void DOGI::populate_go_annotations(const QVector<GFFFile> &files) {
  for (const auto &[database, path] : files)
    populate_go_annotations(path, database);
}

void DOGI::populate_go_annotations(const QString &file_name,
                                   const QString &id_database = {}) {

  logger->info(" [Populate] [GOAnnotations] Engaged ({})",
               id_database.toStdString());

  logger->info(" [Populate] [GOAnnotations] Cleaning");

  if (id_database.isEmpty())
    exec("DELETE FROM GOAnnotations");
  else
    exec("DELETE FROM GOAnnotations WHERE id_database = '" + id_database + "'");

  logger->info(" [Populate] [GOAnnotations] Parsing File {}",
               file_name.toStdString());

  DOGIFile file{file_name};

  file.skipLines('!');

  size_t counter{0};

  QSet<QPair<QString, int>> added{};

  do {
    if (++counter % 50000 == 0)
      cout << counter << endl;

    const auto &data = file.line.split("\t");

    const auto &id_uniprot = data[1];
    const auto &id_go = data[4].mid(3).toInt();

    if (added.contains({id_uniprot, id_go}))
      continue;

    added.insert({id_uniprot, id_go});

    for (const auto &[id_database, id_feature] :
         DOGIFetch::getIDsFromUniprot(*db, id_uniprot, id_database))
      DOGIGO::insertGOAnnotation(*db, id_database, id_feature, id_go);

  } while (file.readLineInto());

  logger->info(" [Populate] [GOAnnotations] Completed");
}

void DOGI::populateGOAnnotations(const QString &file_name) {

  logger->info(" [Populate] [GOAnnotations] Engaged");

  transaction();
  logger->info(" [Populate] [GOAnnotations] Cleaning");

  //  if (id_database.isEmpty())
  exec("DELETE FROM GOAnnotations");
  //  else
  //    exec("DELETE FROM GOAnnotations WHERE id_database = '" + id_database +
  //    "'");

  logger->info(" [Populate] [GOAnnotations] Parsing File {}",
               file_name.toStdString());

  DOGIFile file{file_name};

  file.skipLines('!');

  size_t counter{0};

  QSet<QPair<QString, int>> added{};

  auto fetch_id =
      file_name.endsWith(".mgi")
          ? static_cast<QVector<IDFeature> (*)(
                const QSqlDatabase &, const QString &, const QString &)>(
                &DOGIFetch::getIDsFromMGI)
          : DOGIFetch::getIDsFromUniprot;

  do {
    if (++counter % 50000 == 0)
      cout << counter << endl;

    const auto &data = file.line.split("\t");

    const auto &id_gene = data[1];
    const auto &id_go = extractIntID(data[4]);

    if (added.contains({id_gene, id_go}))
      continue;

    added.insert({id_gene, id_go});

    for (const auto &[id_database, id_feature] : fetch_id(*db, id_gene, ""))
      DOGIGO::insertGOAnnotation(*db, id_database, id_feature, id_go);

  } while (file.readLineInto());

  commit();

  logger->info(" [Populate] [GOAnnotations] Completed");
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

void DOGI::populate_taxon(const QString &path) {

  DOGIFile file{path};
  QString line{};

  QVariantList taxon_id, taxon_name;
  QVariantList other_id, other_name, other_type;

  logger->info("[Populate] [Taxons] Collecting names from file");

  //  while (in.readLineInto(&line) && counter < 100) {
  while (file.readLineInto(&line)) {
    auto data = line.split("\t");
    data.removeAll("|");

    switch (data.size()) {
    case 4:
      if (data.at(3) == "scientific name") {
        taxon_id << data.at(0).toInt();
        taxon_name << (data.at(2).isEmpty() ? data.at(1) : data.at(2));
      } else {
        other_id << data.at(0).toInt();
        other_name << (data.at(2).isEmpty() ? data.at(1) : data.at(2));
        other_type << data.at(3);
      }
      break;
    case 3:
      if (data.at(2) == "scientific name") {
        taxon_id << data.at(0).toInt();
        taxon_name << data.at(1);
      } else {
        other_id << data.at(0).toInt();
        other_name << data.at(1);
        other_type << data.at(2);
      }
      break;
    default:
      throw_error("Unsupported number of columns: " + QString(data.size()));
    }

    if (taxon_id.size() >= 20000)
      break;
  }

  logger->info("[Populate] [Taxons] Inserting Taxon");

  QSqlQuery query(*db);

  prepare(query, "INSERT INTO Taxon (id_taxon, name) VALUES (?, ?)");
  query.addBindValue(taxon_id);
  query.addBindValue(taxon_name);

  execBatch(query);
  query.finish();

  logger->info("[Populate] [Taxons] Inserting Other Names");

  prepare(query, "INSERT INTO TaxonAlias (id_taxon, alias, type) "
                 "VALUES (?, ?, ?)");

  query.addBindValue(other_id);
  query.addBindValue(other_name);
  query.addBindValue(other_type);

  logger->info("[Populate] [Taxons] [INSERT OtherNames] Execute");

  execBatch(query);
  query.finish();

  mark_table("Taxon", path);
  logger->info("[Populate] [Taxons] Finished ({})", taxon_id.size());
}

void DOGI::populate_taxon(const QVector<QPair<int, QString>> taxons,
                          const QVector<QPair<QString, int>> taxon_aliases) {
  QVariantList taxons_id, names;

  taxons_id.reserve(taxons.size());
  names.reserve(taxons.size());

  for (const auto &[id, name] : taxons) {
    taxons_id.append(id);
    names.append(name);
  }

  QSqlQuery query(*db);

  prepare(query, "INSERT INTO DOGITaxons (id_taxon, name) VALUES (?, ?)");

  query.addBindValue(taxons_id);
  query.addBindValue(names);

  execBatch(query);
  query.finish();

  taxons_id.clear();
  names.clear();

  taxons_id.reserve(taxon_aliases.size());
  names.reserve(taxon_aliases.size());

  for (const auto &[name, id] : taxon_aliases) {

    names.append(name);
    taxons_id.append(id);
  }

  prepare(query, "INSERT INTO DOGITaxonAliases (id_alias, id_taxon) "
                 "VALUES (?, ?)");

  query.addBindValue(names);
  query.addBindValue(taxons_id);

  execBatch(query);
  query.finish();
}

void DOGI::populate(const QString &config) {

  logger->info("[Populate] Engaged ({})", config.toStdString());

  QSqlQuery(*this->db);

  auto conf = DOGISnack(config);

  if (conf.isEmpty())
    throw_error("Nothing to populate from: " + config);

  if (const auto &organism = get_id_taxon(conf.organism);
      taxon_id && taxon_id != organism.first)
    throw_error(conf.organism);
  else {
    update_taxon(organism);
  }

  for (const auto &ele : conf.provided)
    cout << ele.toStdString() << endl;

  try {
    populate_annotations(conf.annotations);

    //    if (conf.contains("taxon"))
    //      populate_taxon(conf.taxonomy);

    //    if (conf.contains("ncbi2ensembl"))
    //      populate_ncbi2ensembl(conf.ncbi2ensembl);

    if (conf.contains("hpa_expression"))
      populate_hpa_tissue_expression(conf.hpa_expression);

    if (conf.contains("mapper"))
      populate_mapper(conf.mappers);

    if (conf.contains("go_terms"))
      populate_go_terms(conf.go_terms);

    if (conf.contains("go_annotations"))
      populateGOAnnotations(conf.go_annotations);

    if (conf.contains("genome"))
      populate_genomes(conf.genomes);

    if (conf.contains("structural_variants"))
      populateVarStructural(conf.structural);

    if (conf.contains("regulatory"))
      populateRegulatory(conf.regulatory);

  } catch (const runtime_error &ex) {
    this->db->rollback();
    throw ex;
  }

  logger->info("[Populate] Commiting");

  logger->info("[Populate] Completed");
}
