#include "dogitoys.h"

using namespace DOGIToys;
using namespace DOGITools;

void DOGI::insert_GFF3SystemID(const QString &system,
                               const QString &description) {
  QSqlQuery insert(*db);

  prepare(insert,
          "INSERT OR IGNORE INTO GFF3SystemIDs (id_system, system_description) "
          "VALUES(:system, :description)");
  insert.bindValue(":system", system);
  insert.bindValue(":description", description.size()
                                       ? description
                                       : QVariant(QVariant::String));

  exec(insert);
}

void DOGI::insert_GFF3FeatureAlias(const QString &database, int id_feature,
                                   const QString &alias) {
  QSqlQuery insert(*db);

  prepare(insert, "INSERT INTO GFF3FeatureAliases "
                  "(id_database, id_feature, feature_alias)"
                  "VALUES (:database, :id_feature, :alias)");

  insert.bindValue(":database", database);
  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":alias", gff3_str_clean(alias));

  exec(insert);
}

void DOGI::insert_GFF3FeatureID(const QString &database, int id_feature,
                                const QString &system, const QString &value) {
  QSqlQuery insert(*db);

  prepare(insert, "INSERT INTO GFF3FeatureIDs "
                  "(id_database, id_feature, id_system, feature_idx) "
                  "VALUES (:database, :id_feature, :id_system, :value)");

  insert.bindValue(":database", database);
  insert.bindValue(":id_feature", id_feature);

  if (auto id_system = gff3_str_clean(system);
      id_system == "miRBase" and value.startsWith("MIMAT")) {
    insert.bindValue(":id_system", id_system + "MIMAT");
    insert_GFF3SystemID(id_system + "MIMAT", "");
  } else {
    insert.bindValue(":id_system", id_system);
    insert_GFF3SystemID(id_system, "");
  }

  insert.bindValue(":value", gff3_str_clean(value));

  try {
    exec(insert);
  } catch (const runtime_error &ex) {
    cout << ex.what() << endl;
    cout << database.toStdString() << " " << id_feature << " "
         << system.toStdString() << " " << value.toStdString() << endl;
    throw_error(system);
  }
}

void DOGI::process_GFF3Comment(const QString &line, const QString &database) {
  if (line.startsWith("##sequence-region")) {
    const auto data{line.split(" ", QString::SkipEmptyParts)};
    insert_GFF3SeqID(database, data[1], data[2].toInt(), data[3].toInt());
  }
}

void DOGI::insert_GFF3Feature(const QString &database, int id_feature,
                              const QString &seqid, const QString &source,
                              const QString &type, int first, int last,
                              const QString &score, const QString &strand,
                              const QString &phase) {
  QSqlQuery insert(*db);

  QString query{"INSERT INTO GFF3Features "
                "(id_database, id_feature,"
                "feature_seqid, feature_type, feature_source, "
                "feature_start, feature_end, feature_length, "
                "feature_score, feature_strand, feature_phase) "
                "VALUES (:database, :id_feature, "
                ":seqid, :type, :source, "
                ":first, :last, :length, :score, "
                ":strand, :phase)"};

  prepare(insert, query);

  insert.bindValue(":database", database);
  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":seqid", seqid);
  insert.bindValue(":type", type);
  insert.bindValue(":source",
                   source == "." ? QVariant(QVariant::String) : source);
  insert.bindValue(":first", first);
  insert.bindValue(":last", last);
  insert.bindValue(":length", last - first + 1);
  insert.bindValue(":score", score == "." ? QVariant(QVariant::Double)
                                          : score.toDouble());
  insert.bindValue(":strand",
                   strand == "." ? QVariant(QVariant::String) : strand);
  insert.bindValue(":phase",
                   phase == "." ? QVariant(QVariant::Int) : phase.toInt());

  exec(insert);

  if (type == "chromosome" or type == "region" or type == "contig" or
      type == "supercontig") {
    update_GFF3SeqID(database, seqid, id_feature);
    UpdateTableField("GFF3Features", database, id_feature, "feature_signature",
                     seqid);
  }
}

void DOGI::insert_GFF3FeatureAttribute(const QString &database, int id_feature,
                                       const QString &attr,
                                       const QString &value) {
  QSqlQuery insert(*db);

  prepare(insert, "INSERT INTO GFF3FeatureAttributes "
                  "(id_database, id_feature, "
                  "attribute_name, attribute_value)"
                  "VALUES (:database, :id_feature, :attr, :value)");

  insert.bindValue(":database", database);
  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":attr", attr);
  insert.bindValue(":value", gff3_str_clean(value));

  exec(insert);
}

void DOGI::process_GFF3FeatureAttributes(const QString &database,
                                         int id_feature,
                                         const QString &feature_type,
                                         QHash<QString, int> &parents,
                                         const QString attributes) {

  for (const auto &field : attributes.split(";")) {
    if (const auto sep = field.indexOf('='); sep == -1)
      throw runtime_error(field.toStdString());
    else {
      const auto &attr = field.left(sep);
      const auto &value = field.mid(sep + 1);

      if (attr.length() == 0 or value.length() == 0)
        throw runtime_error(field.toStdString());

      insert_GFF3FeatureAttribute(database, id_feature, attr, value);

      if (attr == "ID")
        parents.insert(value, id_feature);
      else if (attr == "Parent")
        UpdateTableField("GFF3Features", database, id_feature,
                         "feature_id_parent", parents.value(value));
      else if (attr == "biotype" or attr == "gene_biotype")
        UpdateTableField("GFF3Features", database, id_feature,
                         "feature_biotype", value);
      else if (attr == "gene_id" or attr == "transcript_id" or
               attr == "protein_id") {
        insert_GFF3FeatureID(database, id_feature, attr, value);
        UpdateTableField("GFF3Features", database, id_feature,
                         "feature_signature_clean", chop(value, '.'));
        UpdateTableField("GFF3Features", database, id_feature,
                         "feature_signature", value);
      } else if (attr == "Name")
        UpdateTableField("GFF3Features", database, id_feature, "feature_name",
                         value);
      else if (attr == "Dbxref") {
        for (const auto &ele : value.split(",")) {
          if (const auto &sep = ele.indexOf(":"); sep == -1)
            throw runtime_error(value.toStdString());
          else if (ele.startsWith("taxon"))
            continue;
          else {
            const auto &system = ele.left(sep);
            const auto &value = ele.mid(sep + 1);

            insert_GFF3FeatureID(database, id_feature, system, value);
            if (system == "GeneID" &&
                (feature_type == "gene" || feature_type == "pseudogene")) {
              UpdateTableField("GFF3Features", database, id_feature,
                               "feature_signature", value);
              UpdateTableField("GFF3Features", database, id_feature,
                               "feature_signature_clean", chop(value, '.'));
            }
          }
        }
      } else if (attr == "gene_synonym" or attr == "Alias") {
        for (const auto &alias : value.split(",")) {
          insert_GFF3FeatureAlias(database, id_feature, gff3_str_clean(alias));
        }
      } else if (value.indexOf(',') != -1 and attr != "end_range" and
                 attr != "start_range" and attr != "transl_except")
        return;
      //        throw runtime_error(database.toStdString() + " " +
      //        field.toStdString());
    }
  }
}

void DOGI::insert_GFF3Mapper(const QString database_from,
                             const QString &gene_from,
                             const QString &transcript_from,
                             const QString &protein_from,
                             const QString &database_to, const QString &gene_to,
                             const QString &transcript_to,
                             const QString &protein_to) {

  if (transcript_from == "-" or transcript_to == "-")
    return;

  QSqlQuery insert(*db);

  prepare(insert, "INSERT INTO GFF3Mapper"
                  "(id_database_from, gene_from, transcript_from, protein_from,"
                  "id_database_to, gene_to, transcript_to, protein_to)"
                  "VALUES(:data_from, :gene_from, :trans_from, :prot_from, "
                  ":data_to, :gene_to, :trans_to, :prot_to)");

  insert.bindValue(":data_from", database_from);
  insert.bindValue(":gene_from", gene_from);
  insert.bindValue(":trans_from", transcript_from);
  insert.bindValue(":prot_from", protein_from == "-"
                                     ? QVariant(QVariant::String)
                                     : protein_from);
  insert.bindValue(":data_to", database_to);
  insert.bindValue(":gene_to", gene_to);
  insert.bindValue(":trans_to", transcript_to);
  insert.bindValue(":prot_to",
                   protein_to == "-" ? QVariant(QVariant::String) : protein_to);

  try {
    exec(insert);
  } catch (const runtime_error &ex) {
    cout << ex.what() << endl;
    throw_error(database_from + " " + gene_from + " " + database_to + " " +
                gene_to);
  }
}

void DOGI::insert_GFF3FeatureParent(const QString &database, int id_feature,
                                    const QString &parent) {
  QSqlQuery select(*db);

  prepare(select, "SELECT id_feature FROM GFF3Features "
                  "WHERE id_database = :database AND feature_id = :parent");

  select.bindValue(":database", database);
  select.bindValue(":parent", parent);

  exec(select);

  if (!select.next())
    throw_error(database + " " + parent);

  UpdateTableField("GFF3Features", database, id_feature, "feature_id_parent",
                   select.value(0));
}

void DOGI::insert_GFF3FeatureName(const QString &database, int id_feature,
                                  const QString &name) {
  QSqlQuery insert(*db);

  prepare(insert, "INSERT INTO GFF3FeatureNames "
                  "(id_database, id_feature, feature_name) "
                  "VALUES(:database, :id_feature, :name)");

  insert.bindValue(":database", database);
  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":name", gff3_str_clean(name));

  exec(insert);
}

void DOGI::update_GFF3SeqID(const QString &database, const QString &name,
                            int id_feature) {
  QSqlQuery update(*db);

  prepare(update, "UPDATE GFF3SeqIDs SET id_feature = :id_feature "
                  "WHERE id_database = :database AND seqid_name = :name");

  update.bindValue(":id_feature", id_feature);
  update.bindValue(":database", database);
  update.bindValue(":name", name);

  exec(update);
}

void DOGI::insert_GFF3SeqID(const QString &database, const QString &name,
                            int first, int last) {
  QSqlQuery insert(*db);
  prepare(insert, "INSERT INTO GFF3SeqIDs "
                  "(id_database, seqid_name, "
                  "seqid_start, seqid_end, seqid_length)"
                  "VALUES (:database, :name, :first, :last, :length)");

  insert.bindValue(":database", database);
  insert.bindValue(":name", name);
  insert.bindValue(":first", first);
  insert.bindValue(":last", last);
  insert.bindValue(":length", last - first + 1);

  exec(insert);
}
