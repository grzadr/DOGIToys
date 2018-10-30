#include "dogitoys-fetch.h"
#include "dogitoys-tools.h"

using namespace DOGIToys;
using namespace DOGITools;

// QVector<QPair<QString, int>>
int DOGIFetch::getMaxIDFeature(const QSqlDatabase &db, const QString &table,
                               const QString &id_database) {
  if (!db.tables().contains(table))
    throw_error(table);

  QSqlQuery query(db);

  prepare(query, "SELECT MAX(id_feature) FROM " + table +
                     " WHERE id_database = :id_database");

  query.bindValue(":id_database", id_database);

  exec(query);

  query.next();

  if (auto result = query.value(0); result.isNull())
    return 0;
  else
    return result.toInt();
}

QVector<IDFeature> DOGIFetch::getIDsFromSignature(const QSqlDatabase &db,
                                                  const QString &id_database,
                                                  const QString &signature,
                                                  const QString &table,
                                                  bool clean) {
  if (!db.tables().contains(table))
    throw_error(table);

  QSqlQuery query(db);

  QVector<IDFeature> result{};

  QString command{"SELECT id_database, id_feature FROM " + table +
                  " WHERE id_database = :id_database AND "
                  "feature_signature = :signature"};

  if (clean)
    command += " UNION "
               ""
               "SELECT id_database, id_feature FROM " +
               table +
               " WHERE id_database = :id_database"
               " AND feature_signature_clean = :signature_clean";

  prepare(query, command);

  query.bindValue(":id_database", id_database);
  query.bindValue(":signature", signature);
  query.bindValue(":signature_clean", chop(signature, '.'));

  exec(query);

  while (query.next())
    result.append({query.value(0).toString(), query.value(1).toInt()});

  return result;
}

opt_idf DOGIFetch::getIDFromSignature(const QSqlDatabase &db,
                                      const QString &id_database,
                                      const QString &signature,
                                      const QString &table, bool clean) {
  if (!db.tables().contains(table))
    throw_error(table);

  QSqlQuery query(db);

  QString command{"SELECT id_database, id_feature FROM " + table +
                  " WHERE id_database = :id_database AND "
                  "feature_signature = :signature"};

  if (clean)
    command += " UNION "
               ""
               "SELECT id_database, id_feature FROM " +
               table +
               " WHERE id_database = :id_database "
               " AND feature_signature_clean = :signature_clean";

  prepare(query, command);

  query.bindValue(":id_database", id_database);
  query.bindValue(":signature", signature);
  query.bindValue(":signature_clean", chop(signature, '.'));

  cout << "Checking: '" << signature.toStdString() << "'" << endl;

  exec(query);

  if (!query.next())
    return nullopt;

  IDFeature result{query.value(0).toString(), query.value(1).toInt()};

  if (query.next())
    throw_error("Signature not unique '" + signature + "'");

  return result;
}

QVector<IDFeature> DOGIFetch::getIDsFromUniprot(const QSqlDatabase &db,
                                                const QString &id_uniprot,
                                                const QString &id_database) {
  QSqlQuery query(db);

  QString command{"SELECT id_database, id_feature FROM MapUniprot "
                  "WHERE id_uniprot = :id_uniprot"};

  if (id_database.size())
    command += " AND id_database = :id_database";

  prepare(query, command);

  query.bindValue(":id_uniprot", id_uniprot);
  query.bindValue(":id_database", id_database);

  exec(query);

  QVector<IDFeature> result{};

  while (query.next())
    result.append({query.value(0).toString(), query.value(1).toInt()});

  return result;
}

optional<IDFeature> DOGIFetch::selectIDParent(const QSqlDatabase &db,
                                              const QString &id_database,
                                              int id_feature) {
  QSqlQuery query(db);

  prepare(query, "SELECT id_database, feature_id_parent FROM GFF3Features "
                 "WHERE id_database = :id_database AND "
                 "id_feature = :id_feature");

  query.bindValue(":id_database", id_database);
  query.bindValue(":id_feature", id_feature);

  exec(query);

  if (!query.next())
    throw_error("No parent" + id_database + " " + QString::number(id_feature));

  if (query.value(1).isNull())
    return nullopt;
  else
    return IDFeature{query.value(0).toString(), query.value(1).toInt()};
}

optional<IDFeature> DOGIFetch::selectIDParent(const QSqlDatabase &db,
                                              const QString &table,
                                              const QString &id_database,
                                              int id_feature) {
  if (!db.tables().contains(table))
    throw_error(table);

  QSqlQuery query(db);

  prepare(query, "SELECT id_database, feature_id_parent "
                 "FROM " +
                     table +
                     " WHERE id_database = :id_database AND "
                     "id_feature = :id_feature");

  query.bindValue(":id_database", id_database);
  query.bindValue(":id_feature", id_feature);

  exec(query);

  if (!query.next())
    throw_error("No parent" + id_database + " " + QString::number(id_feature));

  if (query.value(1).isNull())
    return nullopt;
  else
    return IDFeature{query.value(0).toString(), query.value(1).toInt()};
}

QVector<IDFeature> DOGIFetch::getIDsFromMGI(const QSqlDatabase &db,
                                            const QString &id_mgi,
                                            const QString &id_database) {
  return getIDsFromMGI(db, extractIntID(id_mgi), id_database);
}

QVector<IDFeature> DOGIFetch::getIDsFromMGI(const QSqlDatabase &db, int id_mgi,
                                            const QString &id_database) {
  QSqlQuery query(db);

  QString command{"SELECT id_database, id_feature FROM MapMGI "
                  "WHERE id_mgi = :id_mgi"};

  if (id_database.size())
    command += " AND id_database = :id_database";

  prepare(query, command);

  query.bindValue(":id_mgi", id_mgi);
  query.bindValue(":id_database", id_database);

  exec(query);

  QVector<IDFeature> result{};

  while (query.next())
    result.append({query.value(0).toString(), query.value(1).toInt()});

  return result;
}

void DOGIFetch::insertMapGFF3(const QSqlDatabase &db,
                              const QString &id_database_from,
                              int id_feature_from,
                              const QString &id_database_to, int id_feature_to,
                              bool strict, bool with_parent) {
  //  if (transcript_from == "-" or transcript_to == "-")
  //    return;

  QSqlQuery insert(db);

  prepare(insert, "INSERT INTO MapGFF3"
                  "(id_database_from, id_feature_from,"
                  " id_database_to, id_feature_to) "
                  "VALUES(:id_database_from, :id_feature_from, "
                  ":id_database_to, :id_feature_to)");

  insert.bindValue(":id_database_from", id_database_from);
  insert.bindValue(":id_feature_from", id_feature_from);
  insert.bindValue(":id_database_to", id_database_to);
  insert.bindValue(":id_feature_to", id_feature_to);

  try {
    exec(insert);
  } catch (const runtime_error &ex) {
    if (strict)
      throw_error(QString::fromLatin1(ex.what()) + "\n" + id_database_from +
                  " " + QString::number(id_feature_from) + " " +
                  id_database_to + " " + QString::number(id_feature_to));
  }

  if (with_parent) {
    const auto &parent_from =
        DOGIFetch::selectIDParent(db, id_database_from, id_feature_from);
    const auto &parent_to =
        DOGIFetch::selectIDParent(db, id_database_to, id_feature_to);

    if (parent_from.has_value() and parent_to.has_value())
      try {
        insertMapGFF3(db, (*parent_from).id_database, (*parent_from).id_feature,
                      (*parent_to).id_database, (*parent_to).id_feature);
      } catch (const runtime_error &) {
      }
  }
}

void DOGIFetch::deleteFeatures(const QSqlDatabase &db, const QString &table,
                               const QString &id_database) {
  QSqlQuery query(db);

  if (!db.tables().contains(table))
    throw_error(table);

  QString command = "DELETE FROM " + table;

  if (id_database.size())
    command += " WHERE id_database = :id_database";

  prepare(query, command);

  query.bindValue(":id_database", id_database);

  exec(query);
}
