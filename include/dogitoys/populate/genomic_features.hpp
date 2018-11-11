#pragma once

#include <QSqlDatabase>
#include <QString>

#include <dogitoys/query.hpp>
#include <hkl/gff.hpp>

namespace DOGIToys::Populate {

using namespace HKL;

void insert_SeqID(QSqlDatabase &db, const QString &name, int first, int last);

class GenomicFeature {
 private:
  QSqlDatabase &db;
  GFF::GFFRecord record;
  int id_feature{0};

  void insert_feature();
  void insert_attributes();
  void insert_alias(const string &aliases);

 public:
  GenomicFeature() = delete;
  GenomicFeature(QSqlDatabase &db, GFF::GFFRecord record)
      : db{db}, record{record} {}

  int insert();
};

}  // namespace DOGIToys::Populate
