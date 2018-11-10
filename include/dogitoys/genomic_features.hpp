#pragma once

#include <QSqlDatabase>
#include <QString>

#include <dogitoys/query.hpp>
#include <hkl/gff.hpp>

namespace DOGIToys::Populate {

using namespace HKL;

void insert_SeqID(QSqlDatabase &db, const QString &name, int first, int last,
                  char strand = 0);

class GenomicFeature {
 private:
  GFF::GFFRecord record;

  void insert_feature(QSqlDatabase &db);

 public:
  GenomicFeature() = delete;
  GenomicFeature(GFF::GFFRecord record) : record{record} {}

  void insert(QSqlDatabase &db);
};

}  // namespace DOGIToys::Populate
