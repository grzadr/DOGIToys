#include <dogitoys/connect.hpp>

using namespace DOGIToys;

DOGI::DOGI(const QString &path, const QString &config) : DOGI() {
  if (config.isEmpty())
    open(path, false);
  else {
    open(path, true);
    //    populate(config);
  }
}

void DOGI::open(const QString &path, bool create) {
  if (this->isOpen())
    throw runerror(
        "DOGI is already connected.\n"
        "Close current connection, before opening a new one.");

  if (create) {
    QFile::remove(path);
  } else {
    if (QFileInfo::exists(path))
      throw_runerror("File doesn't exists\nPath: '" + path + "'");
  }

  this->db_file.setFile(path);

  qWarning() << db_file.absoluteFilePath() << db_file.path() << path;

  db = make_unique<QSqlDatabase>(
      QSqlDatabase::addDatabase(driver, db_file.absoluteFilePath()));
  db->setDatabaseName(db_file.absoluteFilePath());

  if (!db->open())
    throw_runerror("Could not connect to '" + path +
                   "'\nQSqlError: " + this->lastErrorText());
}
