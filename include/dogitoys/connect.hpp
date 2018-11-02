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

#include <dogitoys/query.hpp>
#include "dogitoys/initiate.hpp"
#include "dogitoys/tools.hpp"

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

  Initiate::Initializer initializer{nullptr};

  inline const static QStringList sqlite_opening{"PRAGMA encoding = 'UTF-8';",
                                                 "PRAGMA foreign_keys = 1;"};

  inline const static QStringList sqlite_closing{
      "PRAGMA foreign_key_check;",
      "PRAGMA integrity_check;",
  };

  inline const static QStringList sqlite_optimize{"PRAGMA optimize;"};

  void open_sqlite() { this->exec(sqlite_opening); }
  void close_sqlite(bool optimize = true) {
    exec(sqlite_closing);
    if (optimize) exec(sqlite_optimize);
  }

 public:
  DOGI() = default;
  DOGI(const QString &path, const QString &config = "");
  DOGI(const string &path, const string &config = "")
      : DOGI(QString::fromStdString(path), QString::fromStdString(config)) {}
  DOGI(const char *path, const char *config = "")
      : DOGI(QString::fromLatin1(path), QString::fromLatin1(config)) {}

  ~DOGI() { this->close(false); }

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

  void transaction() { Execute::transaction(*db); }
  void commit() { Execute::commit(*db); }
  void rollback(bool force = false) { Execute::rollback(*db, force); }

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

  void clear_taxon();

  int getIdTaxon() { return this->id_taxon; }
  string getTaxonName() {
    qWarning() << Select::selectIdTaxon(*db).value_or(0);
    return this->taxon_name;
  }

  void setTaxon();
  void setTaxon(int name, bool overwrite);
  void setTaxon(QString name);
  void setTaxon(string name) { setTaxon(QString::fromStdString(name)); }

  void close(bool optimize = false);
};

}  // namespace DOGIToys
