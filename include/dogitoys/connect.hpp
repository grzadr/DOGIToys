#pragma once

#include <QtDebug>

#include <QFileInfo>
#include <QStringList>

#include <QSqlDatabase>
#include <QSqlError>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <dogitoys/populate/populate.hpp>
#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

namespace DOGIToys {

using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using runerror = std::runtime_error;
using std::nullopt;
using std::optional;

class DOGI {
 private:
  shared_ptr<QSqlDatabase> db{nullptr};
  QFileInfo db_file{};
  const QString driver{"QSQLITE"};
  int id_taxon{0};
  string taxon_name{};

  Populate::Populator populator{};

  inline const static QStringList sqlite_opening{"PRAGMA encoding = 'UTF-8';",
                                                 "PRAGMA foreign_keys = 1;"};

  inline const static QStringList sqlite_closing{

  };

  inline const static QStringList sqlite_optimize{
      "PRAGMA optimize;",
  };

  inline const static QStringList sqlite_integrity{
      "PRAGMA foreign_key_check;",
      "PRAGMA integrity_check;",
  };

  void open_sqlite() { this->exec(sqlite_opening); }
  void close_sqlite(bool integrity_check = true, bool optimize = true) {
    exec(sqlite_closing);
    if (integrity_check) exec(sqlite_integrity);
    if (optimize) exec(sqlite_optimize);
  }

 public:
  DOGI() = default;
  DOGI(const QString &path, const QString &config = "");
  DOGI(const string &path, const string &config = "")
      : DOGI(QString::fromStdString(path), QString::fromStdString(config)) {}
  DOGI(const char *path, const char *config = "")
      : DOGI(QString::fromLatin1(path), QString::fromLatin1(config)) {}

  ~DOGI() { this->close(false, false); }

  void open(const QString &path, bool create = false);
  void open(const string &path, bool create = false) {
    this->open(QString::fromStdString(path), create);
  }
  void open(const char *path, bool create = false) {
    this->open(QString::fromLatin1(path), create);
  }

  bool isOpen() { return this->db != nullptr && this->db->isOpen(); }
  QSqlError lastError() { return this->db->lastError(); }
  QString lastErrorText() { return this->db->lastError().text(); }

  void transaction() { Transaction::transaction(*db); }
  void commit() { Transaction::commit(*db); }
  void rollback(bool force = false) { Transaction::rollback(*db, force); }

  QSqlQuery prepare(const QString &query) {
    return Execute::prepare(*this->db, query);
  }

  template <class Query>
  void exec(const Query &query) {
    Execute::exec(*db, query);
  }
  void exec(QSqlQuery &query) { Execute::exec(query); }

  void execBatch(QSqlQuery &queries) { Execute::execBatch(queries); }

  void vacuum() { exec("VACUUM"); }

  void destroy(bool confirm = false);

  void close(bool integrity_check = true, bool optimize = true);

  void clear_taxon();

  int getIdTaxon() { return this->id_taxon; }
  string getTaxonName() { return this->taxon_name; }

  void setTaxon();
  void setTaxon(int name, bool overwrite);
  void setTaxon(QString name, bool overwrite = false);
  void setTaxon(string name, bool overwrite = false) {
    setTaxon(QString::fromStdString(name), overwrite);
  }

  void populateGenomes(QString database, QString fasta_file);
  void populateGenomes(string database, string fasta_file) {
    populateGenomes(QString::fromStdString(database),
                    QString::fromStdString(fasta_file));
  }

  void populateGenomicFeatures(QString gff3_file, bool initiate = true);
  void populateGenomicFeatures(string gff3_file, bool initiate = true) {
    populateGenomicFeatures(QString::fromStdString(gff3_file), initiate);
  }

  void populateGenomicSequences(QString fasta_file, QString masking,
                                bool overwrite = true);
  void populateGenomicSequences(string fasta_file, string masking,
                                bool overwrite = true) {
    populateGenomicSequences(QString::fromStdString(fasta_file),
                             QString::fromStdString(masking), overwrite);
  }

  void populateUniprotMap(QString map_file, bool overwrite = true);
  void populateUniprotMap(string map_file, bool overwrite = true) {
    populateUniprotMap(QString::fromStdString(map_file), overwrite);
  }
};

}  // namespace DOGIToys
