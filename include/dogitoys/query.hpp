#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <optional>

#include <dogitoys/tools.hpp>

namespace DOGIToys {

using std::nullopt;
using std::optional;

namespace Execute {

inline void transaction(QSqlDatabase &db) {
  if (!db.transaction())
    throw_runerror("New transaction failed:\n" + db.lastError().text());
}

inline void commit(QSqlDatabase &db) {
  if (!db.commit())
    throw_runerror("Commiting failed:\n" + db.lastError().text());
}

inline void rollback(QSqlDatabase &db, bool force = false) {
  if (!db.rollback() and !force)
    throw_runerror("Rollback failed:\n" + db.lastError().text());
}

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

inline optional<int> selectIdTaxon(const QSqlDatabase &db) {
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

inline int selectIdTaxon(const QSqlDatabase &db, const QString &organism) {
  auto taxon_query =
      Execute::prepare(db,
                       "SELECT id_taxon FROM DOGITaxons T "
                       "WHERE CAST(id_taxon AS TEXT) = ? OR taxon_name = ? "
                       "OR id_taxon = (SELECT id_taxon "
                       "FROM DOGITaxonAliases WHERE id_alias = ?)");
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  taxon_query.addBindValue(organism);
  Execute::exec(taxon_query);

  if (!taxon_query.next())
    throw_runerror("Organism is not supported: " + organism);

  return taxon_query.value(0).toInt();
}

inline QString selectTaxonName(const QSqlDatabase &db, const int id_taxon) {
  auto taxon_query = Execute::prepare(
      db, "SELECT taxon_name FROM DOGITaxons T WHERE id_taxon = ?");
  taxon_query.addBindValue(id_taxon);
  Execute::exec(taxon_query);

  if (!taxon_query.next())
    throw_runerror("Organism is not supported: " + QString::number(id_taxon));

  return taxon_query.value(0).toString();
}
}  // namespace Select

namespace Update {

inline void UpdateIdTaxon(const QSqlDatabase &db, const int id_taxon) {
  auto update = Execute::prepare(db,
                                 "UPDATE DOGIMaster SET value = ? "
                                 "WHERE id_field = 'id_taxon'");
  update.addBindValue(id_taxon);
  Execute::exec(update);
}

}  // namespace Update
}  // namespace DOGIToys
