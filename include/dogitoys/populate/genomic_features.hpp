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

  void bindMainValues(QSqlQuery &query, const QStringList &fields) {
    for (const auto &ele : fields) {
      if (ele == "id")
        query.bindValue(":id", getID());
      else if (ele == "seqid")
        query.bindValue(":seqid", getSeqID());
      else if (ele == "source")
        query.bindValue(":source", getSource());
      else if (ele == "type")
        query.bindValue(":type", getType());
      else if (ele == "start")
        query.bindValue(":start", getStart());
      else if (ele == "end")
        query.bindValue(":end", getEnd());
      else if (ele == "length")
        query.bindValue(":length", getLength());
      else if (ele == "score")
        query.bindValue(":score", getScore());
      else if (ele == "strand")
        query.bindValue(":strand", getStrand());
      else if (ele == "phase")
        query.bindValue(":phase", getPhase());
      else if (ele == "parent")
        query.bindValue(":parent", getParent());
      else if (ele == "name")
        query.bindValue(":name", getName());
    }
  }

public:
  QGFFRecord() = delete;
  QGFFRecord(GFF::GFFRecord record);
  virtual ~QGFFRecord() = default;

  int getRecordID() const { return id_record; }
  QString getID() const { return QString::fromStdString(*record.at("ID")); }
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

// Old implementation
// class GenomicFeature {
// private:
//  QSqlDatabase &db;
//  GFF::GFFRecord record;
//  int id_feature{0};

//  void insert_feature();
//  void insert_attributes();
//  void insert_alias(const string &aliases);

// public:
//  GenomicFeature() = delete;
//  GenomicFeature(QSqlDatabase &db, GFF::GFFRecord record);

//  void insert(int id_record);
//};

class GenomicFeature : public QGFFRecord {
private:
  inline const static QStringList obligatory_fields{
      "id",  "seqid",  "source", "type",      "start",
      "end", "length", "strand", "signature", "study"};

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
