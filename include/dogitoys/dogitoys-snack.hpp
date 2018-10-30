#pragma once

#include <QVector>

#include <QString>
#include <QStringList>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#include <QtDebug>

using std::cout;
using std::endl;
using std::make_unique;
using std::nullopt;
using std::optional;
using std::unique_ptr;
using opt_qstr = optional<QString>;

using std::runtime_error;

namespace DOGIToys {

struct GFFFile {
  QString database, path;
};

struct Mapper {
  QString from, to, path;
};

struct Genome {
  QString database, masking, path;
};

struct MGIOntology {
  QString ontology, mapper;
};

class DOGISnack {
private:
  [[noreturn]] static inline void throw_error(const QString message) {
    throw runtime_error("DOGISnackError: " + message.toStdString());
  }

  opt_qstr loadFile(QString field, bool obligatory = true);
  optional<QVector<GFFFile>> loadAnnotation(QString field,
                                            bool obligatory = true);
  optional<QVector<Mapper>> loadMapper(QString field, bool obligatory = true);
  optional<QVector<Genome>> loadGenome(QString field, bool obligatory = true);

  //  void load_taxonomy(const QJsonObject &conf);
  void load_annotations(const QJsonObject &conf);
  void loadAnnotations();
  void loadTaxonomy();
  void loadNCBI2Ensembl();
  void loadHPAExpression();
  void loadGOTerms();
  void loadGOAnnotations();
  void loadMappers();
  void loadGenomes();
  void loadGVF();
  void loadStructural();
  void loadRegulatory();

public:
  QJsonObject conf{};

  QStringList provided{};
  QString organism{};
  QFile ncbi2ensembl{};

  QVector<GFFFile> annotations{};
  QVector<GFFFile> gvf{};
  QVector<GFFFile> regulatory{};
  QVector<GFFFile> structural{};

  QString go_terms{};
  QString hpa_expression{};
  QString taxonomy{};
  QString go_annotations{};

  QVector<Mapper> mappers{};
  QVector<Genome> genomes{};

  DOGISnack() = delete;
  DOGISnack(const QString &path);

  auto cbegin() const { return provided.cbegin(); }
  auto cend() const { return provided.cend(); }
  auto begin() const { return provided.begin(); }
  auto end() const { return provided.end(); }

  bool isEmpty() { return provided.isEmpty(); }

  bool contains(const QString &field) { return provided.contains(field); }
};
} // namespace DOGIToys
