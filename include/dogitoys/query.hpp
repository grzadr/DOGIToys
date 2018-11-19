#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDebug>

#include <optional>

#include <dogitoys/tools.hpp>

namespace DOGIToys {

using std::nullopt;
using std::optional;

namespace Transaction {

inline void transaction(QSqlDatabase &db) {
  qInfo() << "New transaction";
  if (!db.transaction())
    throw_runerror("New transaction failed:\n" + db.lastError().text());
}

inline void commit(QSqlDatabase &db) {
  qInfo() << "Commiting transaction";
  if (!db.commit())
    throw_runerror("Commiting failed:\n" + db.lastError().text());
}

inline void rollback(QSqlDatabase &db, bool force = false) {
  qInfo() << "Rolling back";
  if (!db.rollback() and !force)
    throw_runerror("Rollback failed:\n" + db.lastError().text());
}

}  // namespace Transaction

namespace Execute {

inline void prepare(QSqlQuery &query, const QString &command) {
  if (!query.prepare(command))
    throw_runerror("Error preparing query:\n" + query.lastQuery() +
                   "\nMessage:\n" + query.lastError().text());
}

inline QSqlQuery prepare(const QSqlDatabase &db, const QString &command) {
  QSqlQuery query(db);
  if (!query.prepare(command))
    throw_runerror("Error preparing query:\n" + query.lastQuery() +
                   "\nMessage:\n" + query.lastError().text());
  return query;
}

inline void execBatch(QSqlQuery &queries) {
  if (!queries.execBatch())
    throw_runerror("Error executing query:\n" + queries.lastQuery() + "\n" +
                   queries.lastError().text());
}

inline void exec(QSqlQuery &query) {
  if (!query.exec())
    throw_runerror("Error executing query:\n" + query.lastQuery() + "\n" +
                   query.lastError().text());
}

inline void exec(const QSqlDatabase &db, const QString &query) {
  if (auto result = db.exec(query); !result.isActive())
    throw_runerror("Error executing query:\n" + query + "\n" +
                   result.lastError().text());
}

inline void exec(const QSqlDatabase &db, const QStringList &queries) {
  for (const auto &query : queries) exec(db, query);
}

}  // namespace Execute

namespace Select {

inline optional<int> select_id_taxon(const QSqlDatabase &db) {
  auto select = Execute::prepare(
      db, "SELECT value FROM DOGIMaster WHERE id_field = 'id_taxon'");
  Execute::exec(select);

  if (!select.next())
    throw_runerror("Encountered error while getting id_taxon:\n" +
                   select.lastError().text());

  if (auto value = select.value(0); value.isNull())
    return nullopt;
  else
    return value.toInt();
}

inline int select_id_taxon(const QSqlDatabase &db, const QString &organism) {
  auto taxon_query =
      Execute::prepare(db,
                       "SELECT id_taxon FROM Taxons T "
                       "WHERE CAST(id_taxon AS TEXT) = ? OR taxon_name = ? "
                       "OR id_taxon = (SELECT id_taxon "
                       "FROM TaxonAliases WHERE id_alias = ?)");
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  Execute::exec(taxon_query);

  if (!taxon_query.next())
    throw_runerror("Organism is not supported: " + organism);

  return taxon_query.value(0).toInt();
}

inline QString select_taxon_name(const QSqlDatabase &db, const int id_taxon) {
  auto taxon_query = Execute::prepare(
      db, "SELECT taxon_name FROM Taxons T WHERE id_taxon = ?");
  taxon_query.addBindValue(id_taxon);
  Execute::exec(taxon_query);

  if (!taxon_query.next())
    throw_runerror("Organism is not supported: " + QString::number(id_taxon));

  return taxon_query.value(0).toString();
}

inline int select_max_id(const QSqlDatabase &db, const QString &table,
                         const QString &id_field) {
  auto select = Execute::prepare(db, "SELECT IFNULL(MAX(" + id_field +
                                         "), 0) as id_feature FROM " + table);
  Execute::exec(select);
  if (!select.next())
    throw_runerror("I have no idea what happened ...\n" +
                   db.lastError().text());

  return select.value(0).toInt();
}

inline int select_id(const QSqlDatabase &db, const QString &table,
                     const QString &id_field, const QString &signature_field,
                     const QString &signature) {
  auto select = Execute::prepare(
      db, "SELECT " + id_field + " as id_feature FROM " + table + " WHERE " +
              signature_field + " = :signature");
  select.bindValue(":signature", signature);
  Execute::exec(select);

  if (!select.next())
    return 0;
  else
    return select.value(0).toInt();
}
}  // namespace Select

namespace Update {

inline void update_id_taxon(const QSqlDatabase &db, const int id_taxon) {
  auto update = Execute::prepare(db,
                                 "UPDATE DOGIMaster SET value = ? "
                                 "WHERE id_field = 'id_taxon'");
  update.addBindValue(id_taxon);
  Execute::exec(update);
}

}  // namespace Update

}  // namespace DOGIToys
