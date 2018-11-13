#pragma once

#include <QSqlDatabase>
#include <QtDebug>

#include <memory>

#include <dogitoys/initiate.hpp>
#include <dogitoys/populate/genomic_features.hpp>
#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

#include <hkl/gff.hpp>
#include <hkl/regionseq.hpp>

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
  void initGenomicSequences();
  void initSequences();

  void populateGenomicFeatures(QString gff3_file, bool initiate = false);
  void populateGenomicSequences(QString fasta_file, QString masking,
                                bool initiate = false);

  void populateFASTA(QString fasta_file);
};

}  // namespace DOGIToys::Populate
