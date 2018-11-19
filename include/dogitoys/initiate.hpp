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
};

static inline QStringList Taxons{

    "CREATE TABLE Taxons ("
    "id_taxon INTEGER PRIMARY KEY NOT NULL,"
    "taxon_name TEXT NOT NULL COLLATE NOCASE,"
    ""
    "CONSTRAINT length_name CHECK(LENGTH(taxon_name))"
    ")",

    "CREATE UNIQUE INDEX idx_Taxons_name ON Taxons(taxon_name)",

    "CREATE TABLE TaxonAliases("
    "id_alias TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,"
    "id_taxon INTEGER NOT NULL,"
    ""
    "CONSTRAINT length_id_aliast CHECK(LENGTH(id_alias)),"
    ""
    "CONSTRAINT fk_TaxonAliases "
    "FOREIGN KEY (id_taxon)"
    "REFERENCES Taxons(id_taxon)"
    ")",
};

static inline QVector<QPair<int, QString>> BasicTaxonIDs{
    {9606, "Homo sapiens"},       {9615, "Canis lupus familiaris"},
    {9913, "Bos taurus"},         {9823, "Sus scrofa"},
    {10116, "Rattus norvegicus"}, {10090, "Mus musculus"},
};

static inline QVector<QPair<QString, int>> BasicTaxonAliases{
    {"human", 9606},  {"man", 9606},

    {"dog", 9615},    {"dogs", 9615},   {"Canis familiaris", 9615},

    {"cow", 9913},    {"cattle", 9913},

    {"pig", 9823},

    {"rat", 10116},

    {"mouse", 10090},
};

inline static QStringList SeqIDs{
    "DROP TABLE IF EXISTS SeqIDs",
    "CREATE TABLE SeqIDs ("
    "seqid_name TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,"
    "seqid_start INT NOT NULL,"
    "seqid_end INT NOT NULL,"
    "seqid_length INT NOT NULL,"
    "seqid_strand TEXT DEFAULT NULL,"
    ""
    "CONSTRAINT length_seqid_name CHECK(LENGTH(seqid_name)), "
    ""
    "CONSTRAINT value_seqid_start CHECK(seqid_start > 0), "
    "CONSTRAINT value_seqid_end CHECK(seqid_end >= seqid_start)"
    ")",
};

inline static QString GFFBasic{
    "id_feature INTEGER PRIMARY KEY NOT NULL,"
    "feature_seqid TEXT NOT NULL COLLATE NOCASE,"
    "feature_source TEXT COLLATE NOCASE,"
    "feature_type TEXT NOT NULL COLLATE NOCASE,"
    "feature_start INT NOT NULL,"
    "feature_end INT NOT NULL,"
    "feature_length INTEGER NOT NULL,"
    "feature_score REAL,"
    "feature_strand COLLATE NOCASE,"
    "feature_phase INT,"
    "feature_id_parent INTEGER,"
    "feature_signature TEXT COLLATE NOCASE,"};

inline static QStringList GenomicFeatures{
    "DROP TABLE IF EXISTS GenomicFeatures",
    "CREATE TABLE GenomicFeatures(" + GFFBasic +
        "feature_stable_id TEXT COLLATE NOCASE,"
        "feature_name TEXT COLLATE NOCASE,"
        "feature_biotype TEXT COLLATE NOCASE,"
        ""
        "CONSTRAINT typeof_id_parent "
        "CHECK(feature_id_parent IS NULL OR TYPEOF(feature_id_parent) = "
        "'integer'),"
        "CONSTRAINT typeof_start "
        "CHECK(TYPEOF(feature_start) = 'integer'),"
        "CONSTRAINT typeof_end "
        "CHECK(TYPEOF(feature_end) = 'integer'),"
        "CONSTRAINT typeof_length "
        "CHECK(TYPEOF(feature_length) = 'integer'),"
        ""
        "CONSTRAINT length_type CHECK(LENGTH(feature_type)),"
        "CONSTRAINT length_signature "
        "CHECK(feature_signature IS NULL OR LENGTH(feature_signature)),"
        "CONSTRAINT length_name "
        "CHECK(feature_name IS NULL OR LENGTH(feature_name)),"
        "CONSTRAINT length_biotype "
        "CHECK(feature_biotype IS NULL OR LENGTH(feature_biotype)),"

        "CONSTRAINT value_start CHECK(feature_start > 0),"
        "CONSTRAINT value_end CHECK(feature_end >= feature_start),"
        "CONSTRAINT value_strand "
        "CHECK(feature_strand IS NULL OR feature_strand IN ('+', '-')),"
        ""
        "CONSTRAINT fk_GenomicAnnotation_seqid "
        "FOREIGN KEY (feature_seqid) "
        "REFERENCES SeqIDs (seqid_name) "
        "ON DELETE CASCADE,"
        ""
        "CONSTRAINT fk_Features_id_parent "
        "FOREIGN KEY (feature_id_parent) "
        "REFERENCES GenomicFeatures (id_feature) "
        "ON DELETE CASCADE"
        ""
        ")",

    "CREATE INDEX idx_GenomicFeatures_seqid ON "
    "GenomicFeatures(feature_seqid)",
    "CREATE INDEX idx_GenomicFeatures_source ON "
    "GenomicFeatures(feature_source)",
    "CREATE INDEX idx_GenomicFeatures_type ON "
    "GenomicFeatures(feature_type)",
    "CREATE INDEX idx_GenomicFeatures_start ON "
    "GenomicFeatures(feature_start)",
    "CREATE INDEX idx_GenomicFeatures_end ON "
    "GenomicFeatures(feature_end)",
    "CREATE INDEX idx_GenomicFeatures_name ON "
    "GenomicFeatures(feature_name)",
    "CREATE INDEX idx_GenomicFeatures_signature ON "
    "GenomicFeatures(feature_signature)",
    "CREATE INDEX idx_GenomicFeatures_stable_id ON "
    "GenomicFeatures(feature_stable_id)",
    "CREATE INDEX idx_GenomicFeatures_parent ON "
    "GenomicFeatures(feature_id_parent)",
    "CREATE INDEX idx_GenomicFeatures_biotype ON "
    "GenomicFeatures(feature_biotype)",

    "DROP TABLE IF EXISTS GenomicFeatureAttributes",
    "CREATE TABLE GenomicFeatureAttributes ("
    "id_feature INTEGER NOT NULL,"
    "feature_attr_name TEXT NOT NULL COLLATE NOCASE,"
    "feature_attr_value TEXT NOT NULL COLLATE NOCASE,"
    ""
    "PRIMARY KEY (id_feature, feature_attr_name),"
    ""
    "CONSTRAINT fk_GenomicFeaturesAttributes_id_feature "
    "FOREIGN KEY (id_feature) "
    "REFERENCES GenomicFeatures(id_feature) "
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX idx_GenomicFeatureAttributes_name_value ON "
    "GenomicFeatureAttributes(feature_attr_name, feature_attr_value)",

    //    "DROP TABLE IF EXISTS GenomicFeatureIDs",
    //    "CREATE TABLE GenomicFeatureIDs("
    //    "id_feature INTEGER NOT NULL,"
    //    "id_system TEXT NOT NULL COLLATE NOCASE,"
    //    "feature_idx TEXT NOT NULL COLLATE NOCASE,"
    //    ""
    //    "PRIMARY KEY (id_system, feature_idx, id_feature),"
    //    ""
    //    "CONSTRAINT length_feature_idx_length CHECK(LENGTH(feature_idx)),"
    //    ""
    //    "CONSTRAINT fk_GenomicFeatureIDs_feature "
    //    "FOREIGN KEY (id_feature) "
    //    "REFERENCES GenomicFeatures (id_feature)"
    //    ")",

    "DROP TABLE IF EXISTS GenomicFeatureAliases",
    "CREATE TABLE GenomicFeatureAliases("
    "id_feature INTEGER NOT NULL,"
    "feature_alias TEXT NOT NULL,"
    ""
    "PRIMARY KEY (feature_alias, id_feature),"
    ""
    "CONSTRAINT length_alias CHECK(LENGTH(feature_alias)),"
    ""
    "CONSTRAINT fk_FeatureIDs_feature "
    "FOREIGN KEY (id_feature) "
    "REFERENCES GenomicFeatures (id_feature) "
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX idx_GFF3FeatureAliases_alias "
    "ON GenomicFeatureAliases(feature_alias)",
};

inline static QStringList GenomicSequences{
    "DROP TABLE IF EXISTS GenomicSequences",
    "CREATE TABLE GenomicSequences ("
    "id_sequence TEXT NOT NULL COLLATE NOCASE, "
    "sequence_masking TEXT NOT NULL COLLATE NOCASE, "
    "sequence_seq TEXT NOT NULL COLLATE NOCASE, "
    "sequence_length INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_sequence, sequence_masking), "
    ""
    "CONSTRAINT typeof_length CHECK(TYPEOF(sequence_length) = 'integer'), "
    ""
    "CONSTRAINT values_masking "
    "CHECK(sequence_masking IN ('hard', 'soft', 'none')), "
    "CONSTRAINT length_seq CHECK(LENGTH(sequence_seq)), "
    ""
    "CONSTRAINT fk_Features_seqid "
    "FOREIGN KEY (id_sequence) "
    "REFERENCES SeqIDs (seqid_name) "
    "ON DELETE CASCADE"
    ")",
};

inline static QStringList UniprotMap{
    "DROP TABLE IF EXISTS UniprotMap",
    "CREATE TABLE UniprotMap ("
    "uniprot_xref TEXT NOT NULL COLLATE NOCASE,"
    "id_feature INTEGER NOT NULL, "

    "uniprot_db_name TEXT NOT NULL COLLATE NOCASE, "
    "uniprot_info_type TEXT NOT NULL COLLATE NOCASE, "
    "uniprot_source_identity INTEGER DEFAULT NULL, "
    "uniprot_xref_identity INTEGER DEFAULT NULL, "
    "uniprot_linkage_type TEXT DEFAULT NULL COLLATE NOCASE, "
    ""
    "PRIMARY KEY (uniprot_xref, id_feature),"
    ""
    "CONSTRAINT fk_UniprotMap "
    "FOREIGN KEY (id_feature) "
    "REFERENCES GenomicFeatures (id_feature)"
    ")",

    "CREATE INDEX idx_MapUniprot_uniprot_db_name ON "
    "UniprotMap(uniprot_db_name)",
    "CREATE INDEX idx_MapUniprot_uniprot_info_type ON "
    "UniprotMap(uniprot_info_type)",

    //    "CREATE TABLE MapGFF3 ("
    //    "id_mapping INTEGER PRIMARY KEY NOT NULL,"
    //    "id_database_from TEXT NOT NULL COLLATE NOCASE,"
    //    "id_feature_from INTEGER NOT NULL,"
    //    "id_database_to TEXT NOT NULL COLLATE NOCASE,"
    //    "id_feature_to INTEGER NOT NULL,"
    //    ""
    //    "CONSTRAINT unique_mapping "
    //    "UNIQUE(id_database_from, id_feature_from, id_database_to,
    //    id_feature_to),"
    //    ""
    //    "CONSTRAINT fk_MapGFF3_from "
    //    "FOREIGN KEY (id_database_from, id_feature_from) "
    //    "REFERENCES GFF3Features(id_database, id_feature),"
    //    ""
    //    "CONSTRAINT fk_MapGFF3_to "
    //    "FOREIGN KEY (id_database_to, id_feature_to) "
    //    "REFERENCES GFF3Features(id_database, id_feature)"
    //    ")",
    //    "CREATE INDEX idx_MapGFF3_from ON MapGFF3(id_database_from, "
    //    "id_feature_from)",
    //    "CREATE INDEX idx_MapGFF3_to ON MapGFF3(id_database_to,
    //    id_feature_to)",

};

inline static QStringList MGIMap{
    "DROP TABLE IF EXISTS MGIMap",
    "CREATE TABLE MGIMap ("
    "id_mgi INTEGER NOT NULL COLLATE NOCASE, "
    "id_feature INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_mgi, id_feature),"
    ""
    "CONSTRAINT fk_MGIMap_feature "
    "FOREIGN KEY (id_feature) "
    "REFERENCES GenomicFeatures (id_database, id_feature)"
    ")",

    "CREATE INDEX MGIMap_id_feature ON MGIMap(id_feature)",
};

}  // namespace Schemas

using namespace Execute;

void init_main(QSqlDatabase db);
void init_taxon(QSqlDatabase db, QString taxons = {});
void init_genomic_features(QSqlDatabase db);
void init_genomic_sequences(QSqlDatabase db);
void init_uniprot_map(QSqlDatabase db);
}  // namespace DOGIToys::Initiate
