#include "dogitoys/connect.hpp"

#include <QDir>

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
    qWarning() << "Removing old database";
    QFile::remove(path);
  } else {
    if (!QFileInfo::exists(path))
      throw_runerror("File doesn't exists\nPath: '" + path + "'");
  }

  qInfo() << "Connecting to DOGI ...";

  this->db_file.setFile(path);

  db = make_shared<QSqlDatabase>(
      QSqlDatabase::addDatabase(driver, db_file.absoluteFilePath()));
  db->setDatabaseName(db_file.absoluteFilePath());

  populator.reset(db);

  if (!db->open())
    throw_runerror("Could not connect to '" + path +
                   "'\nQSqlError: " + this->lastErrorText());
  else
    qInfo() << "Connected!";

  if (create) populator.initMain();
}

void DOGI::close(bool optimize) {
  if (isOpen()) {
    qInfo() << "Closing connection";

    populator.reset();

    clear_taxon();
    this->rollback(true);

    close_sqlite(optimize);
    db.reset();
    QSqlDatabase::removeDatabase(db_file.absoluteFilePath());

    qInfo() << "Connection closed";
  }
}

void DOGI::destroy(bool confirm) {
  if (confirm) {
    close();
    qWarning() << "Removing old database";
    QFile::remove(db_file.absoluteFilePath());
  }
}

void DOGI::clear_taxon() {
  id_taxon = 0;
  taxon_name.clear();
}

void DOGI::setTaxon() {
  if (auto value = Select::selectIdTaxon(*db))
    clear_taxon();
  else
    setTaxon(Select::selectTaxonName(*db, *value));
}

void DOGI::setTaxon(int id_taxon, bool overwrite = false) {
  auto taxon_name = Select::selectTaxonName(*db, id_taxon);

  if (const auto selected_id_taxon = Select::selectIdTaxon(*db)) {
    if (overwrite && *selected_id_taxon != id_taxon)
      Update::UpdateIdTaxon(*db, id_taxon);
    else
      throw_runerror("Passed id_taxon deffers from id_taxon in DOGIMaster.");
  } else
    Update::UpdateIdTaxon(*db, id_taxon);

  this->id_taxon = id_taxon;
  this->taxon_name = taxon_name.toStdString();
}

void DOGI::setTaxon(QString organism) {
  setTaxon(Select::selectIdTaxon(*db, organism));
}
