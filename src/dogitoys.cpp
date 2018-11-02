#include "dogitoys.h"

using namespace DOGIToys;

using DOGITools::throw_error;

void DOGI::mark_table(const QString &group, const QString &source) {
  QSqlQuery query(*this->db);

  prepare(query, "UPDATE DOGIMaster SET data=:source WHERE category=:group");
  query.bindValue(":group", group);
  query.bindValue(":source", source);

  exec(query);
}

qpair_intqstr DOGI::get_id_taxon(const QString &organism) {
  QSqlQuery taxon_query(*db);

  prepare(taxon_query, "SELECT id_taxon, name FROM DOGITaxons T "
                       "WHERE CAST(id_taxon AS TEXT) = ? OR name = ? "
                       "OR id_taxon = (SELECT id_taxon "
                       "FROM DOGITaxonAliases WHERE id_alias = ?)");
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  exec(taxon_query);

  if (!taxon_query.next())
    throw_error("Organism is not supported: " + organism);

  return {taxon_query.value(0).toInt(), taxon_query.value(1).toString()};
}

void DOGI::clear_taxon() {
  taxon_id = 0;
  taxon_name.clear();
}

void DOGI::set_taxon(const qpair_intqstr &organism) {
  taxon_id = std::move(organism.first);
  taxon_name = std::move(organism.second);
}

void DOGI::set_taxon() {
  QSqlQuery select(*db);

  prepare(select, "SELECT value FROM DOGIMaster WHERE id_field = 'taxon_id'");
  exec(select);

  if (!select.next())
    throw_error("Fuck you");
  else if (select.value(0).isNull())
    clear_taxon();
  else
    set_taxon(get_id_taxon(select.value(0).toString()));
}

void DOGI::update_taxon(const qpair_intqstr &organism) {
  QSqlQuery update(*db);

  prepare(update, "UPDATE DOGIMaster SET value = :taxon_id "
                  "WHERE id_field = 'taxon_id'");
  update.bindValue(":taxon_id", QString::number(organism.first));
  exec(update);
  update.finish();

  prepare(update, "UPDATE DOGIMaster SET value = :taxon_name "
                  "WHERE id_field = 'taxon_name'");
  update.bindValue(":taxon_name", organism.second);
  exec(update);

  set_taxon(organism);
}

QPair<QString, int> DOGI::getFeatureIdFromSignature(const QString &id_database,
                                                    const QString &signature) {
  QSqlQuery query(*db);

  prepare(query, "SELECT id_database, id_feature "
                 "FROM GFF3Features "
                 "WHERE id_database = :id_database AND "
                 "feature_signature = :signature");

  query.bindValue(":id_database", id_database);
  query.bindValue(":signature", signature);

  exec(query);

  if (!query.next())
    throw_error("Feature not found: " + signature);

  QPair<QString, int> result{query.value(0).toString(), query.value(1).toInt()};

  if (query.next())
    throw_error("Feature not unique " + signature);

  return result;
}

optional<QPair<QString, int>>
DOGI::getIDFromSignature(const QString &id_database, const QString &signature) {
  QSqlQuery query(*db);

  prepare(query, "SELECT id_database, id_feature "
                 "FROM GFF3Features "
                 "WHERE id_database = :id_database AND "
                 "feature_signature = :signature");

  query.bindValue(":id_database", id_database);
  query.bindValue(":signature", signature);

  exec(query);

  if (!query.next())
    return nullopt;

  QPair<QString, int> result{query.value(0).toString(), query.value(1).toInt()};

  if (query.next())
    throw_error("Signature not unique " + signature);

  return result;
}

optional<QPair<QString, int>> DOGI::getIDFeature(const QString &id_system,
                                                 const QString &feature_idx,
                                                 const QString &id_database) {
  QSqlQuery query(*db);

  if (id_database.isEmpty()) {
    prepare(query, "SELECT id_database, id_feature FROM GFF3FeatureIDs "
                   "WHERE id_system = :id_system "
                   "AND feature_idx = :feature_idx");
    query.bindValue(":id_system", id_system);
    query.bindValue(":feature_idx", feature_idx);
  } else {
    prepare(query, "SELECT id_database, id_feature FROM GFF3FeatureIDs "
                   "WHERE id_database = :id_database "
                   "AND id_system = :id_system "
                   "AND feature_idx = :feature_idx");
    query.bindValue(":id_database", id_database);
    query.bindValue(":id_system", id_system);
    query.bindValue(":feature_idx", feature_idx);
  }

  exec(query);

  if (!query.next())
    return nullopt;

  QPair<QString, int> result{query.value(0).toString(), query.value(1).toInt()};

  if (query.next())
    throw_error("Feature IDX not unique " + id_system + " " + feature_idx);

  return result;
}

QVector<QPair<QString, int>>
DOGI::getIDsFromSignature(const QString &id_database,
                          const QString &signature) {
  QSqlQuery query(*db);

  QVector<QPair<QString, int>> result{};

  prepare(query, "SELECT id_database, id_feature "
                 "FROM GFF3Features "
                 "WHERE id_database = :id_database AND "
                 "feature_signature = :signature");

  query.bindValue(":id_database", id_database);
  query.bindValue(":signature", signature);

  exec(query);

  while (query.next())
    result.append({query.value(0).toString(), query.value(1).toInt()});

  return result;
}

void DOGI::init_annotation() {
  const vector<Scheme> schemes{GFF3_Databases, GFF3_SeqIDs, GFF3_Features,
                               GFF3_Mapper};

  for (const auto &[names, queries] : schemes) {
    exec(queries);
    register_table(names);
  }
}

void DOGI::init() {
  logger->info("[Init] Engaged");

  transaction();

  try {
    logger->info("[Init] Main");
    exec(DOGI_main);

    logger->info("[Init] Taxon");
    init_taxon();

    logger->info("[Init] IDSystems");
    exec(IDSystems);

    logger->info("[Init] Annotations");
    init_annotation();

    logger->info("[Init] GeneMapping");
    exec(GeneMapping);

    logger->info("[Init] Expression");
    initExpression();

    logger->info("[Init] Gene Onthology");
    initGeneOntology();

    logger->info("[Init] Genomes");
    initGenomes();

    logger->info("[Init] Variants");
    initVariants();

    logger->info("[Init] Regulation");
    initRegulatory();

  } catch (const runtime_error &ex) {
    throw runtime_error("DOGI initialization failed:\n" + string(ex.what()));
  }

  commit();

  logger->info("[Init] Completed");
}

void DOGI::open(const QString &path, bool replace) {
//  if (this->isOpen())
//    throw runtime_error("DOGI is already connected.\n"
//                        "Close current connection, before opening a new one.");

//  this->db_file.setFileName(path);
//  if (replace)
//    db_file.remove();

//  db_file_info.setFile(db_file);

//  name = "DOGI@" + this->db_file_info.absoluteFilePath();

//  db = make_unique<QSqlDatabase>(QSqlDatabase::addDatabase(driver, name));
//  db->setDatabaseName(db_file_info.absoluteFilePath());
//  logger = spdlog::stdout_color_mt(name.toStdString());

//  if (!db->open())
//    throw runtime_error(
//        "Could not connect to '" + db_file_info.path().toStdString() +
//        "'\nQSqlError: " + db->lastError().text().toStdString());

  try {
    open_sqlite();
  } catch (const runtime_error &ex) {
    throw runtime_error("Error initializing SQLITE connection:\n" +
                        string(ex.what()));
  }

  if (db->tables().isEmpty())
    init();
  else
    set_taxon();
}

void DOGI::close(bool soft) {
  if (isOpen()) {
    logger->info("Closing connection and cleaning up");
    clear_taxon();
    this->rollback(true);

    close_sqlite(soft);
    db.reset();
    QSqlDatabase::removeDatabase(name);
    logger->info("Goodbye");
    logger.reset();

    spdlog::drop(name.toStdString());
  }
}
