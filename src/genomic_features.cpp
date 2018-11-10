#include <dogitoys/genomic_features.hpp>

void DOGIToys::Populate::insert_SeqID(QSqlDatabase &db, const QString &name,
                                      int first, int last, char strand) {
  auto insert =
      Execute::prepare(db,
                       "INSERT INTO SeqIDs "
                       "(seqid_name, seqid_start, seqid_end, seqid_length)"
                       "VALUES (:name, :first, :last, :length)");

  insert.bindValue(":name", name);
  insert.bindValue(":first", first);
  insert.bindValue(":last", last);
  insert.bindValue(":length", last - first + 1);

  Execute::exec(insert);
}

void DOGIToys::Populate::GenomicFeature::insert_feature(QSqlDatabase &db) {
  auto insert = Execute::prepare(db,
                                 "INSERT INTO GenomicFeatures "
                                 "(id_feature,"
                                 "feature_seqid, "
                                 "feature_source, "
                                 "feature_type, "
                                 "feature_start, "
                                 "feature_end, "
                                 "feature_length, "
                                 "feature_score, "
                                 "feature_strand, "
                                 "feature_phase,"
                                 "feature_id_parent,"
                                 "feature_signature,"
                                 "feature_name,"
                                 "feature_biotype) "
                                 "VALUES (:id_feature, "
                                 ":type, "
                                 ":seqid, "
                                 ":source, "
                                 ":first, :last, :length, "
                                 ":score, "
                                 ":strand, "
                                 ":phase,"
                                 ":id_parent,"
                                 ":signature,"
                                 ":name,"
                                 ":biotype) ");

  insert.bindValue(":id_feature", id_feature);
  insert.bindValue(":seqid", seqid);
  insert.bindValue(":type", type);
  insert.bindValue(":source",
                   source == "." ? QVariant(QVariant::String) : source);
}

void DOGIToys::Populate::GenomicFeature::insert(QSqlDatabase &db) {
  insert_feature(db);
}
