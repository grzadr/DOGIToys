#pragma once

#include "dogitoys-fetch.h"
#include "dogitoys-file.h"
#include "dogitoys-tools.h"

#include <QSet>

namespace DOGIToys::DOGIGFF {

class GFFRecord {
protected:
  QString seqid{};
  QString source{};
  QString type{};
  int start{0};
  int end{0};
  int length{0};
  QString score{};
  QString strand{};
  QString phase{};
  QString parent{};
  QString signature{};

  QMap<QString, QString> attributes{};

  GFFRecord(const QString &line);

  void parse(const QString &line);

  void bind(QSqlQuery &query, const QString &id_database, int id_feature) const;

  static inline QStringList gff_fields_basic{
      "id_database",   "id_feature",     "feature_seqid", "feature_source",
      "feature_type",  "feature_start",  "feature_end",   "feature_length",
      "feature_score", "feature_strand", "feature_phase", "feature_signature"};

public:
  bool hasParent() const { return !parent.isEmpty(); }
  const QString &getParent() const { return parent; }
};

class GVFStructural : public GFFRecord {
private:
  QString study{};

  static inline QString queryChild{
      "INSERT INTO VarStructuralChildren (" + gff_fields_basic.join(", ") +
      ", feature_signature_parent, feature_study) VALUES (:" +
      gff_fields_basic.join(", :") + ", :signature_parent, :study)"};

  static inline QString queryParent{
      "INSERT INTO VarStructural (" + gff_fields_basic.join(", ") +
      ", feature_study) VALUES (:" + gff_fields_basic.join(", :") +
      ", :study)"};

public:
  GVFStructural() = delete;

  GVFStructural(const QString line);

  void insert(const QSqlDatabase &db, const QString &id_database,
              int id_feature);

  const QString &getSignature() const { return signature; }
  const QString &getSeqID() const { return seqid; }

  static void updateParent(const QSqlDatabase &db, const IDFeature &parent,
                           const IDFeature &child);
};

class GFFRegulation : public GFFRecord {
private:
  QString description{};

public:
  GFFRegulation() = delete;

  GFFRegulation(const QString line);

  void insert(const QSqlDatabase &db, const QString &id_database,
              int id_feature);

  //  const QString &getSignature() const { return signature; }
  //  const QString &getSeqID() const { return seqid; }
};

} // namespace DOGIToys::DOGIGFF
