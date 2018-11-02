#pragma once

#include <QtDebug>

#include <QFileInfo>
#include <QStringList>

#include <QSqlDatabase>
#include <QSqlError>

#include <memory>
#include <stdexcept>
#include <string>

#include <dogitoys/tools.hpp>

namespace DOGIToys {

using std::make_unique;
using std::string;
using std::unique_ptr;
using runerror = std::runtime_error;

class DOGI {
 private:
  unique_ptr<QSqlDatabase> db{nullptr};
  QFileInfo db_file{};
  const QString driver{"QSQLITE"};
  int taxon_id{0};
  QString taxon_name{};

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

  void transaction() {
    if (!this->db->transaction())
      throw_runerror("New transaction failed:\n" + this->lastErrorText());
  }

  void commit() {
    if (!this->db->commit())
      throw_runerror("Commiting failed:\n" + this->lastErrorText());
  }

  void rollback(bool force = false) {
    if (!this->db->rollback() and !force)
      throw_runerror("Rollback failed:\n" + this->lastErrorText());
  }

  QSqlQuery prepare(const QString &query) {
    return Execute::prepare(*this->db, query);
  }

  template <class Query>
  void exec(const Query &query) {
    Execute::exec(*this->db, query);
  }

  void execBatch(QSqlQuery &queries) { Execute::execBatch(queries); }

  void vacuum() { exec("VACUUM"); }

  void close(bool optimize = false) {
    if (isOpen()) {
      //      logger->info("Closing connection and cleaning up");
      //      clear_taxon();
      this->rollback(true);

      close_sqlite(optimize);
      db.reset();
      QSqlDatabase::removeDatabase(db_file.absoluteFilePath());
    }
  }
};

}  // namespace DOGIToys
