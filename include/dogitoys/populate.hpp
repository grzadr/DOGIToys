#pragma once

#include <QSqlDatabase>
#include <QtDebug>

#include <memory>

#include <dogitoys/genomic_features.hpp>
#include <dogitoys/initiate.hpp>
#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

#include <hkl/gff.hpp>

namespace DOGIToys::Populate {

using std::shared_ptr;

class Populator {
 private:
  shared_ptr<QSqlDatabase> db{nullptr};

 public:
  Populator() = default;
  ~Populator() = default;
  void reset(shared_ptr<QSqlDatabase> db) { this->db = db; }
  void reset() { this->db.reset(); }

  void initMain();
  void initTaxon();
  void initGenomicFeatures();
  void initSequences();

  void populateGenomicFeatures(QString gff3_file);
  void populateFASTA(QString fasta_file);
};

}  // namespace DOGIToys::Populate
