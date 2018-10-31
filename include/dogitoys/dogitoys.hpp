#pragma once

#include <QDebug>

#include <QHash>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

#include <QVariant>

#include <QFileInfo>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <memory>
#include <optional>
#include <stdexcept>

//#include "dogitoys-fetch.h"
//#include "dogitoys-file.h"
//#include "dogitoys-gff.h"
//#include "dogitoys-go.h"
//#include "dogitoys-schemas.h"
//#include "dogitoys-snack.h"
#include "dogitoys/dogitoys-tools.hpp"

using std::make_unique;
using std::nullopt;
using std::unique_ptr;
using qpair_intqstr = QPair<int, QString>;
using runerror = std::runtime_error;

namespace DOGIToys {

class DOGI {
 private:
  // Database handler
  unique_ptr<QSqlDatabase> db{nullptr};

  QString name{};
  inline static const QString driver{"QSQLITE"};
  QFile db_file{};
  //  QFileInfo db_file_info{};
  int taxon_id{0};
  QString taxon_name{};

  //  inline const static QStringList sqlite_opening{"PRAGMA encoding =
  //  'UTF-8';",
  //                                                 "PRAGMA foreign_keys =
  //                                                 1;"};

  //  inline const static QStringList sqlite_closing_soft{
  //      "PRAGMA foreign_key_check;", "PRAGMA integrity_check;"};

  //  inline const static QStringList sqlite_closing{"PRAGMA
  //  foreign_key_check;",
  //                                                 "PRAGMA integrity_check;",
  //                                                 "PRAGMA optimize;"};

  void prepare(QSqlQuery &query, const QString &command) {
    DOGITools::prepare(query, command);
  }

  void execBatch(QSqlQuery &queries) { DOGITools::exec(queries); }
  void exec(QSqlQuery &query) { DOGITools::exec(query); }
  void exec(const QString &query) { DOGITools::exec(*db, query); }
  void exec(const QStringList &queries) { DOGITools::exec(*db, queries); }

  //  void remove_data_table(const QString &name);
  //  void remove_data_table_group(const QString &group);

  void register_table(const QStringList &names);
  void register_table(const QString &name);
  void mark_table(const QString &group, const QString &source);

  bool basic_check(const QString &path);

  void init_taxon();
  void init_annotation();
  void init_expression();

  void register_annotation(const QString &source, const QString &data);

  template <typename T>
  void UpdateTableField(const QString &table, const QString &database,
                        int id_feature, const QString &field, const T &value) {
    QSqlQuery update(*db);

    prepare(update,
            "UPDATE " + table + " SET " + field +
                " = :value "
                "WHERE id_database = :database AND id_feature = :id_feature");

    update.bindValue(":value", value);
    update.bindValue(":database", database);
    update.bindValue(":id_feature", id_feature);

    exec(update);
  }

  qpair_intqstr get_id_taxon(const QString &organism);
  void clear_taxon();
  void set_taxon(const qpair_intqstr &organism);
  void set_taxon();
  void update_taxon(const qpair_intqstr &organism);

  QPair<QString, int> getFeatureIdFromSignature(const QString &id_database,
                                                const QString &signature);

  optional<QPair<QString, int>> getIDFromSignature(const QString &id_database,
                                                   const QString &signature);

  optional<QPair<QString, int>> getIDFeature(const QString &id_system,
                                             const QString &feature_idx,
                                             const QString &id_database = "");

  QVector<QPair<QString, int>> getIDsFromSignature(const QString &id_database,
                                                   const QString &signature);

  void insert_GFF3SystemID(const QString &system, const QString &description);

  void insert_GFF3Mapper(const QString database_from, const QString &gene_from,
                         const QString &transcript_from,
                         const QString &protein_from,
                         const QString &database_to, const QString &gene_to,
                         const QString &transcript_to,
                         const QString &protein_to);

  void insertMapUniprot(const QString &id_database, int id_feature,
                        const QString &id_uniprot,
                        const QString &uniprot_source,
                        const QString &uniprot_type);

  void insert_GFF3FeatureParent(const QString &database, int id_feature,
                                const QString &parent);

  void insert_GFF3FeatureName(const QString &database, int id_feature,
                              const QString &name);

  void insert_GFF3FeatureAlias(const QString &database, int id_feature,
                               const QString &alias);

  void insert_GFF3FeatureID(const QString &database, int id_feature,
                            const QString &system, const QString &value);

  void update_GFF3SeqID(const QString &database, const QString &name,
                        int id_feature);

  void insert_GFF3SeqID(const QString &database, const QString &name, int first,
                        int last);

  void process_GFF3Comment(const QString &line, const QString &database);

  void insert_GFF3Feature(const QString &database, int id_feature,
                          const QString &seqid, const QString &source,
                          const QString &type, int first, int last,
                          const QString &score, const QString &strand,
                          const QString &phase);

  void insert_GFF3FeatureAttribute(const QString &database, int id_feature,

                                   const QString &attr, const QString &value);

  void process_GFF3FeatureAttributes(const QString &database, int id_feature,
                                     const QString &feature_type,
                                     QHash<QString, int> &parents,
                                     const QString attributes);

  void populate_taxon(const QString &path);
  void populate_taxon(const QVector<QPair<int, QString>> taxons,
                      const QVector<QPair<QString, int>> taxon_aliases);

  void populate_annotations(const QVector<GFFFile> &files);
  void populate_ncbi2ensembl(const QString &file);
  void populate_ncbi2ensembl(QFile &file);

  void populate_hpa_tissue_expression(QFile &file);

  void populateMapperNCBI2Ensembl(const QString &file_name);
  void populate_mapper_ensembl_uniprot(const QString &file_name);

  void insertMapMGI(int id_mgi, const QString &id_database, int id_feature);

  void insert_chromosome(const QString &id_database, const QString &masking,
                         const QString &id_chrom, const QString &seq);

  void insertGOAnnotation(const QString &id_database, int id_feature,
                          int id_go);

  void close_sqlite(bool soft = false) {
    if (!soft) exec(sqlite_closing);
  }
  void open_sqlite() { this->exec(sqlite_opening); }

 public:
  void open(const QString &path, const QString &config) {
    if (!config.isEmpty()) {
      open(path, true);
      populate(config);
    } else
      open(path, false);
  }
  void open(const string &path, const string &config) {
    this->open(QString::fromStdString(path), QString::fromStdString(config));
  }
  void open(const char *path, const char *config) {
    this->open(QString::fromLatin1(path), QString::fromLatin1(config));
  }

  void open(const char *path, const QString &config) {
    this->open(QString::fromLatin1(path), config);
  }

  ~DOGI() { this->close(true); }

  /*! \brief Open connection to database file.
   * Function responsible for connecting to DOGI database. Function checks if
   * DOGi object has other connection opened. If so it throws an
   * runtime_error. Next it checks if database file exists. If it does not
   * will create it
   */

  // Purpose:
  // Description:
  //// TO DO
  // Requirements:
  //   1) No other connection can be opened.
  //   2) File must already exist for this functions to work.
  // Exceptions:
  //   1) DOGI is already connected to other database file -> runtime_error

  // Purpose: Creation of a new database file.
  // Description:
  //// TO DO
  // Requirements:
  //   Permisions to write files.
  // Exceptions:
  //   1) Can't create database file -> runtime_error.

  void open(const QString &path, bool replace = false);

  void open(const string &path, bool replace = false) {
    this->open(QString::fromStdString(path), replace);
  }
  void open(const char *path, bool replace = false) {
    this->open(QString::fromLatin1(path), replace);
  }

  void close(bool soft = false);

  void init();
  void initGeneOntology() { exec(GeneOntology); }
  void initExpression() { exec(HPA_TissueExpression); }
  void initGenomes() { exec(Genomes); }
  //  void initGVF() { exec(GVF); }
  void initRegulatory() { exec(RegulatoryFeatures); }
  void initVariants() { exec(Variants); }

  void populate(const QString &config);
  void populate(const string &config) {
    this->populate(QString::fromStdString(config));
  }
  void populate(const char *config) {
    this->populate(QString::fromLatin1(config));
  }

  void populate_hpa_tissue_expression(const QString &expression);

  void populate_mapper(QVector<Mapper> &mappers);
  void populate_mapper(const QString &database_from, const QString &database_to,
                       const QString &file_name);
  void populate_go_terms(const QString &file_name, bool init = false);

  void populate_go_annotations(const QVector<GFFFile> &files);

  void populate_go_annotations(const QString &file_name,
                               const QString &id_database);

  void populateGOAnnotations(const QString &file_name);

  void populateMapperMGI(const QString &file_name);

  void populateVarStructural(const QVector<GFFFile> &gvf);
  void populateVarStructural(const QString &id_database,
                             const QString &file_name);

  void populate_genomes(const QString &id_database, const QString &masking,
                        const QString &file_name);
  void populate_genomes(const QVector<Genome> &genomes);

  void populateRegulatory(const QVector<GFFFile> &gff);
  void populateRegulatory(const QString &id_database, const QString &file_name);

  string lastError() { return this->db->lastError().text().toStdString(); }

  //  bool isOpen() { return this->db != nullptr && this->db->isOpen(); }
};

}  // namespace DOGIToys
