#pragma once

#include <QSqlDatabase>
#include <QStringList>
#include <QVector>
#include <QtDebug>

#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

namespace DOGIToys::Initiate {

namespace Schemas {
static inline QStringList DOGI_main{
    // Table of all data tables with information of source
    // and if it was populated
    "CREATE TABLE DOGIMaster ("
    "id_field TEXT NOT NULL COLLATE NOCASE,"
    "value TEXT DEFAULT NULL COLLATE NOCASE,"
    "CONSTRAINT length_DOGIMaster_id_field CHECK(LENGTH(id_field)),"
    "CONSTRAINT length_DOGIMaster_value CHECK(LENGTH(value))"
    ");",

    "INSERT INTO DOGIMaster (id_field) VALUES ('id_taxon');",

    "CREATE TABLE DOGIAnnotations ("
    "source TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,"
    "data TEXT COLLATE NOCASE,"
    ""
    "CONSTRAINT length_DOGIAnnotations_source CHECK(LENGTH(source)),"
    "CONSTRAINT length_DOGIAnnotations_data CHECK(LENGTH(data))"
    ")",

    "CREATE TABLE DOGITaxons ("
    "id_taxon INTEGER PRIMARY KEY NOT NULL,"
    "taxon_name TEXT NOT NULL COLLATE NOCASE,"
    ""
    "CONSTRAINT length_name CHECK(LENGTH(taxon_name))"
    ")",

    "CREATE UNIQUE INDEX idx_DOGITaxons_name ON DOGITaxons(taxon_name)",

    "CREATE TABLE DOGITaxonAliases("
    "id_alias TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,"
    "id_taxon INTEGER NOT NULL,"
    ""
    "CONSTRAINT length_id_aliast CHECK(LENGTH(id_alias)),"
    ""
    "CONSTRAINT fk_TaxonAliases "
    "FOREIGN KEY (id_taxon)"
    "REFERENCES DOGITaxons(id_taxon)"
    ")"};

static inline QVector<QPair<int, QString>> Taxons{
    {9606, "Homo sapiens"},       {9615, "Canis lupus familiaris"},
    {9913, "Bos taurus"},         {9823, "Sus scrofa"},
    {10116, "Rattus norvegicus"}, {10090, "Mus musculus"},
};

static inline QVector<QPair<QString, int>> TaxonAliases{
    {"human", 9606},  {"man", 9606},

    {"dog", 9615},    {"dogs", 9615},   {"Canis familiaris", 9615},

    {"cow", 9913},    {"cattle", 9913},

    {"pig", 9823},

    {"rat", 10116},

    {"mouse", 10090},
};

inline static QStringList Genomes{
    "DROP TABLE IF EXISTS Genomes",
    "CREATE TABLE Genomes ("
    "id_database TEXT NOT NULL COLLATE NOCASE, "
    "masking TEXT NOT NULL COLLATE NOCASE, "
    "id_chrom TEXT NOT NULL COLLATE NOCASE, "
    "seq TEXT NOT NULL COLLATE NOCASE, "
    ""
    "PRIMARY KEY (id_database, id_chrom, masking), "
    ""
    "CONSTRAINT masking_values CHECK(masking IN ('hard', 'soft', 'none')), "
    "CONSTRAINT length_seq CHECK(LENGTH(seq)), "
    ""
    "CONSTRAINT fk_id_database "
    "FOREIGN KEY (id_database) "
    "REFERENCES GFF3Databases(id_database)"
    "ON DELETE CASCADE, "
    ""
    "CONSTRAINT fk_Features_seqid "
    "FOREIGN KEY (id_database, id_chrom) "
    "REFERENCES GFF3SeqIDs (id_database, seqid_name) "
    "ON DELETE CASCADE"
    ")"};

}  // namespace Schemas

using namespace Execute;

void init_main(QSqlDatabase db);
void init_taxon(QSqlDatabase db, QString taxons = {});
}  // namespace DOGIToys::Initiate
