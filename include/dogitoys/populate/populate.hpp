#pragma once

#include <QSqlDatabase>
#include <QtDebug>

#include <memory>

#include <dogitoys/initiate.hpp>
#include <dogitoys/populate/gene_ontology.hpp>
#include <dogitoys/populate/genomic_features.hpp>
#include <dogitoys/populate/mapping.hpp>
#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

#include <agizmo/files.hpp>

#include <hkl/gff.hpp>
#include <hkl/regionseq.hpp>

namespace DOGIToys::Populate {

using std::shared_ptr;

class Populator {
 private:
  shared_ptr<QSqlDatabase> db{nullptr};

  void initTaxon();
  void initGenomicFeatures();
  void initGenomicSequences();
  void initUniprotMap();
  void initMGIMap();
  void initGeneOntology();

  void transaction() const { Transaction::transaction(*db); }
  void commit() const { Transaction::commit(*db); }

  void populateGeneOntologyHierarchy(const QVector<QPair<int, int>> hierarchy);
  void populateGeneOntologyAnnotation(const QString mgi_file, bool overwrite,
                                      bool from_mgi);

 public:
  Populator() = default;
  ~Populator() = default;
  void reset(shared_ptr<QSqlDatabase> db) { this->db = db; }
  void reset() { this->db.reset(); }

  void initMain();

  void populateGenomicFeatures(QString gff3_file, bool initiate);
  void populateGenomicSequences(QString fasta_file, QString masking,
                                bool initiate);
  void insertGenomicSequence(const HKL::RegionSeq& seq, const QString masking);

  void populateUniprotMap(const QString map_file, bool overwrite);
  void populateMGIMap(const QString map_file, bool overwrite);

  void populateGeneOntologyTerms(const QString& obo_file, bool overwrite);
  void populateGeneOntologyAnnotation(const QString file_name, bool overwrite);
};

}  // namespace DOGIToys::Populate
