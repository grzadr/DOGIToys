#pragma once

#include <QtDebug>

#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <exception>
#include <iostream>
#include <optional>

#include <agizmo/files.hpp>

#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

namespace DOGIToys::GeneOntology {

using qvec_int = QVector<int>;

class GeneOntologyTerm {
 private:
  int id{0};
  int replaced_by{0};
  bool is_obsolete{false};
  QString name{};
  QString names{};
  QString def{};
  QString comment{};
  QStringList synonyms{};
  qvec_int is_a{};
  qvec_int consider{};
  qvec_int intersection_of{};
  qvec_int relationship{};
  qvec_int alt_id{};

 public:
  void parse(const QString &line);
  void insert(QSqlDatabase &db);

  bool isObsolete() const { return is_obsolete; }

  void setID(const QString &id);

  int getID() const { return id; }

  void setName(const QString &name);

  QString getName() const { return name; }

  void setNamespace(const QString &names);

  QString getNamespace() const { return names; }

  void setDef(const QString &def);

  QString getDef() const { return def; }

  void addSynonym(const QString &synonym);

  void addIsA(const QString &is_a);

  const qvec_int &getIsA() const { return is_a; }
  bool hasParents() const { return !is_a.isEmpty(); }

  void setIsObsolete(const QString &obsolete);

  void setComment(const QString &comment);

  QString getComment() const { return comment; }

  void addConsider(const QString &consider);

  void addIntersectionOf(const QString &intersection_of);

  void addRelationship(const QString &relationship);

  void setReplacedBy(const QString &replaced_by);

  void addAltID(const QString &alt_id);

  const qvec_int &getAltID() const { return alt_id; }
};

class OBOParser {
 private:
  QFileReader reader;

 public:
  //  QVector<DOGIGOTerm> parse();

  OBOParser() = delete;

  OBOParser(const QString &obo_name) : reader{obo_name} {}

  ~OBOParser() = default;

  std::optional<GeneOntologyTerm> getTerm();
  std::optional<GeneOntologyTerm> operator()() { return getTerm(); }
};

void insert_go_hierarchy(const QSqlDatabase &db, int id_go, int go_is_a);

class GAFRecord {
 private:
  QString uniprot_xref{};
  int id_go;
  bool from_mgi;

 public:
  GAFRecord() = delete;
  GAFRecord(const QString &line, bool from_mgi);

  void insert(QSqlDatabase &db) const;
};

// void insertGOAnnotation(const QSqlDatabase &db, const QString &id_database,
//                        int id_feature, int id_go);

}  // namespace DOGIToys::GeneOntology
