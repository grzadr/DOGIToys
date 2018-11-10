#pragma once

#include <dogitoys/query.hpp>

#include <QString>

namespace DOGIToys::GFFQueries {

// template <typename T>
// inline void update_field(const QSqlDatabase &db, const QString &table,
//                         int id_feature, const QString &field, const T &value)
//                         {
//  auto update =
//      Execute::prepare(db, "UPDATE " + table + " SET " + field +
//                               " = :value WHERE id_feature = :id_feature");

//  update.bindValue(":value", value);
//  update.bindValue(":id_feature", id_feature);

//  Execute::exec(update);
//}

void DOGIToys::insert_GenomicFeature(int id_feature, const QString &seqid,
                                     const QString &source, const QString &type,
                                     int first, int last, const QString &score,
                                     const QString &strand,
                                     const QString &phase) {
  QSqlQuery insert(*db);

  QString query{
      "INSERT INTO GFF3Features "
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
  insert.bindValue(
      ":score", score == "." ? QVariant(QVariant::Double) : score.toDouble());
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

}  // namespace DOGIToys::GFFQueries
