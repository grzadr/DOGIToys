#pragma once

#include <QSqlDatabase>
#include <QtDebug>

#include <memory>

#include <dogitoys/initiate.hpp>
#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

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
  void init_sequences();

  void populateFASTA(QString fasta_file);
};

}  // namespace DOGIToys::Populate
