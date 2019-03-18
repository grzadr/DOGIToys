#pragma once

#include <QSqlDatabase>
#include <QString>

#include <dogitoys/query.hpp>
#include <hkl/gff.hpp>

namespace DOGIToys::Populate {

using namespace HKL;

void insert_SeqID(QSqlDatabase &db, const QString &name, int first, int last);

class QGFFRecord {
protected:
  int id_record{0};
  GFF::GFFRecord record;

  void bindMainValues(QSqlQuery &query, const QStringList &fields);

public:
  QGFFRecord() = delete;
  QGFFRecord(GFF::GFFRecord record);
  virtual ~QGFFRecord() = default;

  int getRecordID() const { return id_record; }
  //  QString getID() const { return QString::fromStdString(*record.at("ID")); }
  QString getSeqID() const {
    return QString::fromStdString(*record.getSeqID());
  }
  QVariant getSource() const {
    if (const auto &source = record.getSource())
      return QString::fromStdString(*source);
    else
      return QVariant(QVariant::String);
  }
  QVariant getType() const {
    if (const auto &type = record.getType())
      return QString::fromStdString(*type);
    else
      return QVariant(QVariant::String);
  }

  int getStart() const { return record.getStart(); }
  int getEnd() const { return record.getEnd(); }
  int getLength() const { return static_cast<int>(record.getLength()); }
  QVariant getStrand() const {
    if (const auto &strand = record.getStrand())
      return QString::fromStdString(std::string(1, *strand));
    else
      return QVariant(QVariant::String);
  }

  QVariant getScore() const {
    if (const auto score = record.getScore())
      return *score;
    else
      return QVariant(QVariant::Double);
  }

  QVariant getPhase() const {
    if (const auto phase = record.getPhase())
      return *phase;
    else
      return QVariant(QVariant::Int);
  }

  QVariant getParent() const {
    if (const auto &parent = record.get("Parent"))
      return QString::fromStdString(*parent.value());
    else
      return QVariant(QVariant::String);
  }

  bool hasParent() const { return record.has("Parent"); }

  QVariant getName() const {
    if (const auto &parent = record.get("Name"))
      return QString::fromStdString(*parent.value());
    else
      return QVariant(QVariant::String);
  }
};

class GenomicFeature : public QGFFRecord {
private:
  inline const static QStringList obligatory_fields{
      "id",     "seqid", "source", "type",      "start", "end",
      "length", "phase", "strand", "signature", "name"};

  void insertFeature(QSqlDatabase &db);
  void insertAttributes(QSqlDatabase &db);
  void insertAlias(QSqlDatabase &db, const string &aliases);

  void insertChildFeature(QSqlDatabase &db);
  void insertChildAttributes(QSqlDatabase &db);
  void insertChildAlias(QSqlDatabase &db, const string &aliases);

public:
  GenomicFeature() = delete;
  GenomicFeature(GFF::GFFRecord record);

  void insert(QSqlDatabase &db, int id_record);
  void insertChild(QSqlDatabase &db, int id_record);

  QVariant getBiotype() const {
    if (const auto &parent = record.get("biotype"))
      return QString::fromStdString(*parent.value());
    else
      return QVariant(QVariant::String);
  }
};

class StructuralVariant : public QGFFRecord {
private:
  inline const static QStringList obligatory_fields{
      "id",  "seqid",  "source", "type",      "start",
      "end", "length", "strand", "signature", "study"};

  QString signature;

public:
  StructuralVariant() = delete;
  StructuralVariant(GFF::GFFRecord record);

  QVariant getStudy() const {
    if (const auto &study = record.get("study_accession"))
      return QString::fromStdString(*study.value());
    else
      return QVariant(QVariant::String);
  }

  void insert(QSqlDatabase &db, int id_record);
  void insertChild(QSqlDatabase &db, int id_record);
  QString getSignature() const { return signature; }
  void bindStartRange(QSqlQuery &query) const;
  void bindEndRange(QSqlQuery &query) const;
};

} // namespace DOGIToys::Populate
