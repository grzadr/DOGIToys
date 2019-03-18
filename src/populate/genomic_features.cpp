#include <dogitoys/populate/genomic_features.hpp>

#include <iostream>

DOGIToys::Populate::QGFFRecord::QGFFRecord(HKL::GFF::GFFRecord record)
    : record{record} {}

void DOGIToys::Populate::insert_SeqID(QSqlDatabase &db, const QString &name,
                                      int first, int last) {
  auto insert =
      Execute::prepare(db, "INSERT INTO SeqIDs "
                           "(seqid_name, seqid_start, seqid_end, seqid_length)"
                           "VALUES (:name, :first, :last, :length)");

  insert.bindValue(":name", name);
  insert.bindValue(":first", first);
  insert.bindValue(":last", last);
  insert.bindValue(":length", last - first + 1);

  Execute::exec(insert);
}

void DOGIToys::Populate::GenomicFeature::insertFeature(QSqlDatabase &db) {
  auto query = Execute::prepare(db, "INSERT INTO GenomicFeatures ("
                                    "id_feature,"
                                    "feature_seqid, "
                                    "feature_source, "
                                    "feature_type, "
                                    "feature_start, "
                                    "feature_end, "
                                    "feature_length, "
                                    "feature_score, "
                                    "feature_strand, "
                                    "feature_phase,"
                                    "feature_signature,"
                                    "feature_stable_id, "
                                    "feature_name, "
                                    "feature_biotype"
                                    ") "
                                    "VALUES ("
                                    ":id, "
                                    ":seqid, "
                                    ":source, "
                                    ":type, "
                                    ":start, "
                                    ":end, "
                                    ":length, "
                                    ":score, "
                                    ":strand, "
                                    ":phase,"
                                    ":signature,"
                                    ":stable_id, "
                                    ":name, "
                                    ":biotype)");

  bindMainValues(query, obligatory_fields);

  if (const auto signature = record.get("ID")) {
    query.bindValue(":signature", QString::fromStdString(*(*signature)));
    if (const auto found = (*signature).value().find(":"); found)
      query.bindValue(
          ":stable_id",
          QString::fromStdString((*signature).value().substr(found + 1)));
    else
      query.bindValue(":stable_id", QVariant(QVariant::String));
  } else if (const auto &type = record.getType().value();
             type == "chromosome" or type == "region" or type == "contig" or
             type == "supercontig") {
    query.bindValue(":signature", QString::fromStdString(*record.getSeqID()));
    query.bindValue(":stable_id", QString::fromStdString(*record.getSeqID()));
  } else {
    query.bindValue(":signature", QVariant(QVariant::String));
    query.bindValue(":stable_id", QVariant(QVariant::String));
  }

  query.bindValue(":name", getName());

  Execute::exec(query);
}

void DOGIToys::Populate::GenomicFeature::insertAttributes(QSqlDatabase &db) {
  auto insert = Execute::prepare(db, "INSERT INTO GenomicFeatureAttributes "
                                     "(id_feature, "
                                     "feature_attr_name, feature_attr_value) "
                                     "VALUES ("
                                     ":id, :name, :value)");
  for (const auto &[key, value] : record) {
    insert.bindValue(":id", id_record);
    insert.bindValue(":name", QString::fromStdString(key));
    if (value.has_value())
      insert.bindValue(":value", QString::fromStdString(*value));
    else
      insert.bindValue(":value", QVariant(QVariant::String));

    Execute::exec(insert);

    if (key == "Alias" and value.has_value())
      insertAlias(db, *value);
  }
}

void DOGIToys::Populate::GenomicFeature::insertAlias(QSqlDatabase &db,
                                                     const string &aliases) {
  auto insert = Execute::prepare(db, "INSERT INTO GenomicFeatureAliases "
                                     "(id_feature, feature_alias)"
                                     "VALUES (:id, :alias)");

  for (const auto &alias : QString::fromStdString(aliases).split(",")) {
    insert.bindValue(":id", id_record);
    insert.bindValue(":alias", alias);
    Execute::exec(insert);
  }
}

void DOGIToys::Populate::GenomicFeature::insertChildFeature(QSqlDatabase &db) {
  auto query = Execute::prepare(db, "INSERT INTO GenomicFeaturesChildren ("
                                    "id_feature_child,"
                                    "feature_child_seqid, "
                                    "feature_child_source, "
                                    "feature_child_type, "
                                    "feature_child_start, "
                                    "feature_child_end, "
                                    "feature_child_length, "
                                    "feature_child_score, "
                                    "feature_child_strand, "
                                    "feature_child_phase,"
                                    "feature_child_id_parent, "
                                    "feature_child_signature,"
                                    "feature_child_stable_id, "
                                    "feature_child_name, "
                                    "feature_child_biotype"
                                    ") "
                                    "VALUES ("
                                    ":id, "
                                    ":seqid, "
                                    ":source, "
                                    ":type, "
                                    ":start, "
                                    ":end, "
                                    ":length, "
                                    ":score, "
                                    ":strand, "
                                    ":phase,"
                                    ":id_parent, "
                                    ":signature,"
                                    ":stable_id, "
                                    ":name, "
                                    ":biotype)");

  bindMainValues(query, obligatory_fields);

  if (const auto signature = record.get("ID")) {
    query.bindValue(":signature", QString::fromStdString(*(*signature)));
    if (const auto found = (*signature).value().find(":"); found)
      query.bindValue(
          ":stable_id",
          QString::fromStdString((*signature).value().substr(found + 1)));
    else
      query.bindValue(":stable_id", QVariant(QVariant::String));
  } else if (const auto &type = record.getType().value();
             type == "chromosome" or type == "region" or type == "contig" or
             type == "supercontig") {
    query.bindValue(":signature", QString::fromStdString(*record.getSeqID()));
    query.bindValue(":stable_id", QString::fromStdString(*record.getSeqID()));
  } else {
    query.bindValue(":signature", QVariant(QVariant::String));
    query.bindValue(":stable_id", QVariant(QVariant::String));
  }

  query.bindValue(":name", getName());

  const auto parent = *(*record.get("Parent"));

  if (const auto id_parent = Select::select_id(
          db, "GenomicFeatures", "id_feature", "feature_signature",
          QString::fromStdString(parent));
      id_parent != 0)
    query.bindValue(":id_parent", id_parent);
  else if (const auto id_parent = Select::select_id(
               db, "GenomicFeatureChildren", "id_feature_child",
               "feature_child_signature", QString::fromStdString(parent));
           id_parent != 0)
    query.bindValue(":id_parent", id_parent);
  else
    throw_runerror("Parent" + parent + " not found!");

  Execute::exec(query);
}

void DOGIToys::Populate::GenomicFeature::insertChildAttributes(
    QSqlDatabase &db) {
  auto insert =
      Execute::prepare(db, "INSERT INTO GenomicFeatureChildrenAttributes "
                           "(id_feature_child, "
                           "feature_child_attr_name, "
                           "feature_child_attr_value) "
                           "VALUES (:id, :name, :value)");
  for (const auto &[key, value] : record) {
    insert.bindValue(":id", id_record);
    insert.bindValue(":name", QString::fromStdString(key));
    if (value.has_value())
      insert.bindValue(":value", QString::fromStdString(*value));
    else
      insert.bindValue(":value", QVariant(QVariant::String));

    Execute::exec(insert);

    if (key == "Alias" and value.has_value())
      insertAlias(db, *value);
  }
}

void DOGIToys::Populate::GenomicFeature::insertChildAlias(
    QSqlDatabase &db, const string &aliases) {
  auto insert =
      Execute::prepare(db, "INSERT INTO GenomicFeatureChildrenAliases "
                           "(id_feature_child, "
                           "feature_child_alias)"
                           "VALUES (:id, :alias)");

  for (const auto &alias : QString::fromStdString(aliases).split(",")) {
    insert.bindValue(":id", id_record);
    insert.bindValue(":alias", alias);
    Execute::exec(insert);
  }
}

void DOGIToys::Populate::GenomicFeature::insert(QSqlDatabase &db,
                                                int id_record) {
  this->id_record = id_record;
  insertFeature(db);
  insertAttributes(db);
}

void DOGIToys::Populate::GenomicFeature::insertChild(QSqlDatabase &db,
                                                     int id_record) {
  this->id_record = id_record;
  insertChildFeature(db);
  insertChildAttributes(db);
}

DOGIToys::Populate::GenomicFeature::GenomicFeature(HKL::GFF::GFFRecord record)
    : QGFFRecord(record) {}

DOGIToys::Populate::StructuralVariant::StructuralVariant(
    HKL::GFF::GFFRecord record)
    : QGFFRecord(record) {
  this->signature =
      extractID(QString::fromStdString(*record.at("Dbxref")), ':');
}

void DOGIToys::Populate::StructuralVariant::insertChild(QSqlDatabase &db,
                                                        int id_record) {
  auto query = Execute::prepare(db, "INSERT INTO StructuralVariantsChildren ("
                                    "id_struct_child, "
                                    "struct_child_seqid, "
                                    "struct_child_source, "
                                    "struct_child_type, "
                                    "struct_child_start, "
                                    "struct_child_end, "
                                    "struct_child_length, "
                                    "struct_child_strand, "
                                    "struct_child_signature, "
                                    "struct_child_study, "
                                    "struct_child_parent_signature, "
                                    "struct_child_id, "
                                    "struct_child_start_range_start, "
                                    "struct_child_start_range_end, "
                                    "struct_child_end_range_start, "
                                    "struct_child_end_range_end"
                                    ") VALUES ("
                                    ":id_struct, "
                                    ":seqid, "
                                    ":source, "
                                    ":type, "
                                    ":start, "
                                    ":end, "
                                    ":length, "
                                    ":strand, "
                                    ":signature, "
                                    ":study, "
                                    ":parent_signature, "
                                    ":id, "
                                    ":start_range_start, "
                                    ":start_range_end, "
                                    ":end_range_start, "
                                    ":end_range_end"
                                    ")");
  bindMainValues(query, obligatory_fields);
  query.bindValue(":id_struct", id_record);
  query.bindValue(":signature", signature);
  query.bindValue(":study", getStudy());
  query.bindValue(":parent_signature", getParent());
  bindStartRange(query);
  bindEndRange(query);

  Execute::exec(query);
}

void DOGIToys::Populate::StructuralVariant::insert(QSqlDatabase &db,
                                                   int id_record) {

  auto query = Execute::prepare(db, "INSERT INTO StructuralVariants ("
                                    "id_struct, "
                                    "struct_seqid, "
                                    "struct_source, "
                                    "struct_type, "
                                    "struct_start, "
                                    "struct_end, "
                                    "struct_length, "
                                    "struct_strand, "
                                    "struct_signature, "
                                    "struct_study, "
                                    "struct_id, "
                                    "struct_start_range_start, "
                                    "struct_start_range_end, "
                                    "struct_end_range_start, "
                                    "struct_end_range_end"
                                    ") VALUES ("
                                    ":id_struct, "
                                    ":seqid, "
                                    ":source, "
                                    ":type, "
                                    ":start, "
                                    ":end, "
                                    ":length, "
                                    ":strand, "
                                    ":signature, "
                                    ":study, "
                                    ":id, "
                                    ":start_range_start, "
                                    ":start_range_end, "
                                    ":end_range_start, "
                                    ":end_range_end"
                                    ")");
  bindMainValues(query, obligatory_fields);
  query.bindValue(":id_struct", id_record);
  query.bindValue(":signature", signature);
  query.bindValue(":study", getStudy());
  bindStartRange(query);
  bindEndRange(query);

  Execute::exec(query);
}

void DOGIToys::Populate::StructuralVariant::bindStartRange(
    QSqlQuery &query) const {
  if (const auto &range = record.get("Start_range")) {
    const auto &start_end = QString::fromStdString(*range.value()).split(',');
    if (start_end.size() != 2)
      throw_runerror("Start_range of element ID=" + *record.at("ID") +
                     " is not correct");

    query.bindValue(":start_range_start", start_end.first());
    query.bindValue(":start_range_end", start_end.last());
  } else {
    query.bindValue(":start_range_start", QVariant(QVariant::String));
    query.bindValue(":start_range_end", QVariant(QVariant::String));
  }
}

void DOGIToys::Populate::StructuralVariant::bindEndRange(
    QSqlQuery &query) const {
  if (const auto &range = record.get("End_range")) {
    const auto &start_end = QString::fromStdString(*range.value()).split(',');
    if (start_end.size() != 2)
      throw_runerror("End_range of element ID=" + *record.at("ID") +
                     " is not correct");

    query.bindValue(":end_range_start", start_end.first());
    query.bindValue(":end_range_end", start_end.last());
  } else {
    query.bindValue(":end_range_start", QVariant(QVariant::String));
    query.bindValue(":end_range_end", QVariant(QVariant::String));
  }
}
