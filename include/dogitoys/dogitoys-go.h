#pragma once

#include <QtDebug>

#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <exception>
#include <memory>

#include <iostream>

#include "dogitoys-file.h"
#include "dogitoys-tools.h"

using std::cout;
using std::endl;

using std::make_unique;
using std::runtime_error;
using std::unique_ptr;

using qvec_int = QVector<int>;
using qvec_pair_int = QVector<QPair<int, int>>;

namespace DOGIToys {

class DOGIGOTerm {
private:
  int id{0};
  int replaced_by{0};
  bool is_obsolete{false};
  QString name{}, names{}, def{}, comment{};
  QStringList synonyms{};
  QVector<int> is_a{}, consider{}, intersection_of{}, relationship{}, alt_id{};

public:
  DOGIGOTerm() = default;

  static int extractID(const QString &id) {
    return id.mid(id.indexOf(":") + 1).toInt();
  }

  static int extractID(const QString &id, const QChar &before) {
    return extractID(id.left(id.indexOf(before) - 1));
  }

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
  bool hasSlaves() const { return !is_a.isEmpty(); }

  void setIsObsolete(const QString &obsolete);

  void setComment(const QString &comment);

  QString getComment() const { return comment; }

  void addConsider(const QString &consider);

  void addIntersectionOf(const QString &intersection_of);

  void addRelationship(const QString &relationship);

  void setReplacedBy(const QString &replaced_by);

  void addAltID(const QString &alt_id);

  const QVector<int> &getAltID() const { return alt_id; }
};

class DOGIGOParser {
private:
  unique_ptr<DOGIFile> file{nullptr};

public:
  QVector<DOGIGOTerm> parse();

  void setFile(const QString &file_name);

  DOGIGOParser() = delete;

  DOGIGOParser(const QString &file_name) { setFile(file_name); }

  ~DOGIGOParser() = default;
};

namespace DOGIGO {

void insertGOTerm(const QSqlDatabase &db, int id_go, const QString &go_name,
                  const QString &go_namespace, const QString &go_def,
                  const QString &go_comment, bool go_is_obsolete,
                  int go_id_master);

qvec_pair_int insertGOTerm(const QSqlDatabase &db, const DOGIGOTerm &term);

void insertGOHierarchy(const QSqlDatabase &db, int id_go, int go_is_a = 0);

void insertGOAnnotation(const QSqlDatabase &db, const QString &id_database,
                        int id_feature, int id_go);

} // namespace DOGIGO

} // namespace DOGIToys