#pragma once

#include "dogitoys-tools.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <QString>
#include <QStringList>
#include <QVector>

#include <exception>
#include <iostream>
#include <optional>

using std::cout;
using std::endl;
using std::nullopt;
using std::optional;
using std::runtime_error;

struct IDFeature {
  QString id_database;
  int id_feature;

  QString toQString() const {
    return "<" + id_database + "; " + QString::number(id_feature) + ">";
  }
};

using opt_idf = optional<IDFeature>;

namespace DOGIToys {
class DOGIFetch {
public:
  static int getMaxIDFeature(const QSqlDatabase &db, const QString &table,
                             const QString &id_database);

  static QVector<IDFeature> getIDsFromSignature(const QSqlDatabase &db,
                                                const QString &id_database,
                                                const QString &signature,
                                                const QString &table,
                                                bool clean = false);

  static opt_idf getIDFromSignature(const QSqlDatabase &db,
                                    const QString &id_database,
                                    const QString &signature,
                                    const QString &table, bool clean = false);

  static QVector<IDFeature> getIDsFromUniprot(const QSqlDatabase &db,
                                              const QString &id_uniprot,
                                              const QString &id_database = {});

  static optional<IDFeature> selectIDParent(const QSqlDatabase &db,
                                            const QString &id_database,
                                            int id_feature);

  static optional<IDFeature> selectIDParent(const QSqlDatabase &db,
                                            const QString &table,
                                            const QString &id_database,
                                            int id_feature);

  static QVector<IDFeature> getIDsFromMGI(const QSqlDatabase &db,
                                          const QString &id_mgi,
                                          const QString &id_database = {});
  static QVector<IDFeature> getIDsFromMGI(const QSqlDatabase &db, int id_mgi,
                                          const QString &id_database = {});

  static void insertMapGFF3(const QSqlDatabase &db,
                            const QString &id_database_from,
                            int id_feature_from, const QString &id_database_to,
                            int id_feature_to, bool strict = true,
                            bool with_parent = false);

  static void deleteFeatures(const QSqlDatabase &db, const QString &table,
                             const QString &id_database = "");
};
} // namespace DOGIToys
