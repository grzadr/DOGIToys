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

void DOGIToys::Populate::GenomicFeature::insert_feature() {
  id_feature = Select::select_max_id(db, "GenomicFeatures", "id_feature") + 1;

  auto insert =
      Execute::prepare(db, "INSERT INTO GenomicFeatures ("
                           "id_feature,"
                           "feature_seqid, feature_source, feature_type, "
                           "feature_start, feature_end, feature_length, "
                           "feature_score, feature_strand, feature_phase,"
                           "feature_id_parent, feature_signature,"
                           "feature_stable_id, "
                           "feature_name, feature_biotype"
                           ") "
                           "VALUES ("
                           ":id_feature, "
                           ":seqid, :source, :type, "
                           ":start, :end, :length, "
                           ":score, :strand, :phase,"
                           ":id_parent, :signature,"
                           ":stable_id, "
                           ":name, :biotype)");

  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":seqid", QString::fromStdString(*record.getSeqID()));

  if (const auto source = record.getSource())
    insert.bindValue(":source", QString::fromStdString(*source));
  else
    insert.bindValue(":source", QVariant(QVariant::String));

  insert.bindValue(":type", QString::fromStdString(*record.getType()));
  insert.bindValue(":start", record.getStart());
  insert.bindValue(":end", record.getEnd());
  insert.bindValue(":length", static_cast<int>(record.getLength()));

  if (const auto score = record.getScore())
    insert.bindValue(":score", *score);
  else
    insert.bindValue(":score", QVariant(QVariant::Double));

  if (const auto strand = record.getStrand())
    insert.bindValue(":strand",
                     QString::fromStdString(std::string(1, *strand)));
  else
    insert.bindValue(":strand", QVariant(QVariant::String));

  if (const auto phase = record.getPhase())
    insert.bindValue(":phase", *phase);
  else
    insert.bindValue(":phase", QVariant(QVariant::Int));

  if (const auto parent = record.get("Parent")) {
    if (const auto id_parent = Select::select_id(
            db, "GenomicFeatures", "id_feature", "feature_signature",
            QString::fromStdString(*(*parent)));
        id_parent != 0)
      insert.bindValue(":id_parent", id_parent);
    else
      throw_runerror("Parent" + *(*parent) + " not found!");
  } else
    insert.bindValue(":id_parent", QVariant(QVariant::Int));

  if (const auto signature = record.get("ID")) {
    insert.bindValue(":signature", QString::fromStdString(*(*signature)));
    if (const auto found = (*signature).value().find(":"); found)
      insert.bindValue(
          ":stable_id",
          QString::fromStdString((*signature).value().substr(found + 1)));
    else
      insert.bindValue(":stable_id", QVariant(QVariant::String));
  } else if (const auto &type = record.getType().value();
             type == "chromosome" or type == "region" or type == "contig" or
             type == "supercontig") {
    insert.bindValue(":signature", QString::fromStdString(*record.getSeqID()));
    insert.bindValue(":stable_id", QString::fromStdString(*record.getSeqID()));
  } else {
    insert.bindValue(":signature", QVariant(QVariant::String));
    insert.bindValue(":stable_id", QVariant(QVariant::String));
  }

  if (const auto name = record.get("Name"))
    insert.bindValue(":name", QString::fromStdString(*(*name)));
  else
    insert.bindValue(":name", QVariant(QVariant::String));

  if (const auto biotype = record.get("biotype"))
    insert.bindValue(":biotype", QString::fromStdString(*(*biotype)));
  else
    insert.bindValue(":biotype", QVariant(QVariant::String));

  Execute::exec(insert);
}

void DOGIToys::Populate::GenomicFeature::insert_attributes() {
  auto insert = Execute::prepare(db, "INSERT INTO GenomicFeatureAttributes "
                                     "(id_feature, "
                                     "feature_attr_name, feature_attr_value) "
                                     "VALUES ("
                                     ":id_feature, :name, :value)");
  for (const auto &[key, value] : record) {
    insert.bindValue(":id_feature", id_feature);
    insert.bindValue(":name", QString::fromStdString(key));
    if (value.has_value())
      insert.bindValue(":value", QString::fromStdString(*value));
    else
      insert.bindValue(":value", QVariant(QVariant::String));

    Execute::exec(insert);

    if (key == "Alias" and value.has_value())
      insert_alias(*value);
  }
}

void DOGIToys::Populate::GenomicFeature::insert_alias(const string &aliases) {
  auto insert = Execute::prepare(db, "INSERT INTO GenomicFeatureAliases "
                                     "(id_feature, feature_alias)"
                                     "VALUES (:id_feature, :alias)");

  for (const auto &alias : QString::fromStdString(aliases).split(",")) {
    insert.bindValue(":id_feature", id_feature);
    insert.bindValue(":alias", alias);
    Execute::exec(insert);
  }
}

int DOGIToys::Populate::GenomicFeature::insert() {
  insert_feature();
  insert_attributes();
  return id_feature;
}

DOGIToys::Populate::GenomicFeature::GenomicFeature(QSqlDatabase &db,
                                                   HKL::GFF::GFFRecord record)
    : db{db}, record{record} {}

DOGIToys::Populate::StructuralVariant::StructuralVariant(
    HKL::GFF::GFFRecord record, int id_record)
    : QGFFRecord(record) {
  this->id_record = id_record;
  this->signature =
      extractID(QString::fromStdString(*record.at("Dbxref")), ':');
}

void DOGIToys::Populate::StructuralVariant::insert(QSqlDatabase &db) {
  auto query = Execute::prepare(
      db, "INSERT INTO StructuralVariants ("
          "id_struct, struct_seqid, struct_source, struct_type, "
          "struct_start, struct_end, struct_length, struct_strand,"
          "struct_signature, struct_study, "
          "struct_parent_signature,"
          "struct_start_range_start, struct_start_range_end,"
          "struct_end_range_start, struct_end_range_end"
          ") VALUES ("
          ":id, :seqid, :source, :type, "
          ":start, :end, :length, :strand,"
          ":signature, :study, :parent_signature,"
          ":start_range_start, :start_range_end,"
          ":end_range_start, :end_range_end"
          ")");
  bindMainValues(query, obligatory_fields);
  query.bindValue(":signature", signature);
  query.bindValue(":study", getStudy());
  query.bindValue(":parent_signature", getParent());
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
