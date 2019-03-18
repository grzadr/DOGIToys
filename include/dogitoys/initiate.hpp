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

inline static QStringList GenomicFeatures{
    "DROP TABLE IF EXISTS GenomicFeatureChildren",
    "DROP TABLE IF EXISTS GenomicFeatures",

    "CREATE TABLE GenomicFeatures("
    "id_feature INTEGER PRIMARY KEY NOT NULL,"
    "feature_seqid TEXT NOT NULL COLLATE NOCASE,"
    "feature_source TEXT COLLATE NOCASE,"
    "feature_type TEXT NOT NULL COLLATE NOCASE,"
    "feature_start INT NOT NULL,"
    "feature_end INT NOT NULL,"
    "feature_length INTEGER NOT NULL,"
    "feature_score REAL DEFAULT NULL,"
    "feature_strand TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_phase INTEGER DEFAULT NULL,"
    //    "feature_id_parent INTEGER,"
    "feature_signature TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_stable_id TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_name TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_biotype TEXT DEFAULT NULL COLLATE NOCASE,"
    ""
    //    "CONSTRAINT typeof_id_parent "
    //    "CHECK(feature_id_parent IS NULL OR TYPEOF(feature_id_parent) = "
    //    "'integer'),"
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
    "ON DELETE CASCADE"
    ""
    ")",

    "CREATE INDEX idx_GenomicFeatures_loc ON "
    "GenomicFeatures(feature_seqid, feature_start, feature_end)",
    "CREATE INDEX idx_GenomicFeatures_source ON "
    "GenomicFeatures(feature_source)",
    "CREATE INDEX idx_GenomicFeatures_type ON "
    "GenomicFeatures(feature_type)",
    //    "CREATE INDEX idx_GenomicFeatures_start ON "
    //    "GenomicFeatures(feature_start)",
    //    "CREATE INDEX idx_GenomicFeatures_end ON "
    //    "GenomicFeatures(feature_end)",
    "CREATE INDEX idx_GenomicFeatures_name ON "
    "GenomicFeatures(feature_name)",
    "CREATE INDEX idx_GenomicFeatures_signature ON "
    "GenomicFeatures(feature_signature)",
    "CREATE INDEX idx_GenomicFeatures_stable_id ON "
    "GenomicFeatures(feature_stable_id)",
    //    "CREATE INDEX idx_GenomicFeatures_parent ON "
    //    "GenomicFeatures(feature_id_parent)",
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

    "CREATE TABLE GenomicFeatureChildren("
    "id_feature_child INTEGER PRIMARY KEY NOT NULL,"
    "feature_child_seqid TEXT NOT NULL COLLATE NOCASE,"
    "feature_child_source TEXT COLLATE NOCASE,"
    "feature_child_type TEXT NOT NULL COLLATE NOCASE,"
    "feature_child_start INT NOT NULL,"
    "feature_child_end INT NOT NULL,"
    "feature_child_length INTEGER NOT NULL,"
    "feature_child_score REAL,"
    "feature_child_strand TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_child_phase INT,"
    "feature_child_id_parent INTEGER NOT NULL,"
    "feature_child_signature TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_child_parent_signature TEXT DEFAULT NULL COLLATE NOCASE,"
    "feature_child_stable_id TEXT COLLATE NOCASE,"
    "feature_child_name TEXT COLLATE NOCASE,"
    "feature_child_biotype TEXT COLLATE NOCASE,"
    ""
    "CONSTRAINT typeof_child_id_parent "
    "CHECK(TYPEOF(feature_child_id_parent) = 'integer'),"
    "CONSTRAINT typeof_child_start "
    "CHECK(TYPEOF(feature_child_start) = 'integer'),"
    "CONSTRAINT typeof_child_end "
    "CHECK(TYPEOF(feature_child_end) = 'integer'),"
    "CONSTRAINT typeof_child_length "
    "CHECK(TYPEOF(feature_child_length) = 'integer'),"
    ""
    "CONSTRAINT length_child_type CHECK(LENGTH(feature_child_type)),"
    "CONSTRAINT length_child_signature "
    "CHECK(feature_child_signature IS NULL OR LENGTH(feature_child_signature)),"
    "CONSTRAINT length_child_name "
    "CHECK(feature_child_name IS NULL OR LENGTH(feature_child_name)),"
    "CONSTRAINT length_child_biotype "
    "CHECK(feature_child_biotype IS NULL OR LENGTH(feature_child_biotype)),"

    "CONSTRAINT value_child_start CHECK(feature_child_start > 0),"
    "CONSTRAINT value_child_end CHECK(feature_child_end >= "
    "feature_child_start),"
    "CONSTRAINT value_child_strand "
    "CHECK(feature_child_strand IS NULL OR feature_child_strand IN ('+', '-')),"
    ""
    "CONSTRAINT fk_GenomicAnnotation_seqid "
    "FOREIGN KEY (feature_child_seqid) "
    "REFERENCES SeqIDs (seqid_name) "
    "ON DELETE CASCADE,"
    ""
    "CONSTRAINT fk_Features_id_parent "
    "FOREIGN KEY (feature_child_id_parent) "
    "REFERENCES GenomicFeatures (id_feature) "
    "ON DELETE CASCADE"
    ""
    ")",

    "CREATE INDEX idx_GenomicFeatureChildren_loc ON "
    "GenomicFeatureChildren(feature_child_seqid, feature_child_start, "
    "feature_child_end)",
    "CREATE INDEX idx_GenomicFeatureChildren_source ON "
    "GenomicFeatureChildren(feature_child_source)",
    "CREATE INDEX idx_GenomicFeatureChildren_type ON "
    "GenomicFeatureChildren(feature_child_type)",
    //    "CREATE INDEX idx_GenomicFeatures_start ON "
    //    "GenomicFeatures(feature_child_start)",
    //    "CREATE INDEX idx_GenomicFeatures_end ON "
    //    "GenomicFeatures(feature_child_end)",
    "CREATE INDEX idx_GenomicFeatureChildren_name ON "
    "GenomicFeatureChildren(feature_child_name)",
    "CREATE INDEX idx_GenomicFeatureChildren_signature ON "
    "GenomicFeatureChildren(feature_child_signature)",
    "CREATE INDEX idx_GenomicFeatureChildren_stable_id ON "
    "GenomicFeatureChildren(feature_child_stable_id)",
    "CREATE INDEX idx_GenomicFeatureChildren_parent ON "
    "GenomicFeatureChildren(feature_child_id_parent)",
    "CREATE INDEX idx_GenomicFeatureChildren_biotype ON "
    "GenomicFeatureChildren(feature_child_biotype)",

    "DROP TABLE IF EXISTS GenomicFeatureChildrenAttributes",
    "CREATE TABLE GenomicFeatureChildrenAttributes ("
    "id_feature_child INTEGER NOT NULL,"
    "feature_child_attr_name TEXT NOT NULL COLLATE NOCASE,"
    "feature_child_attr_value TEXT NOT NULL COLLATE NOCASE,"
    ""
    "PRIMARY KEY (id_feature_child, feature_child_attr_name),"
    ""
    "CONSTRAINT fk_GenomicFeatureChildrenAttributes_id_feature "
    "FOREIGN KEY (id_feature_child) "
    "REFERENCES GenomicChildrenFeatures(id_feature_child) "
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX idx_GenomicFeatureChildrenAttributes_name_value ON "
    "GenomicFeatureChildrenAttributes(feature_child_attr_name, "
    "feature_child_attr_value)",

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

    "DROP TABLE IF EXISTS GenomicFeatureChildrenAliases",
    "CREATE TABLE GenomicFeatureChildrenAliases("
    "id_feature_child INTEGER NOT NULL,"
    "feature_child_alias TEXT NOT NULL,"
    ""
    "PRIMARY KEY (feature_child_alias, id_feature_child),"
    ""
    "CONSTRAINT length_alias CHECK(LENGTH(feature_child_alias)),"
    ""
    "CONSTRAINT fk_FeatureIDs_feature "
    "FOREIGN KEY (id_feature_child) "
    "REFERENCES GenomicFeatureChildren (id_feature_child) "
    "ON DELETE CASCADE"
    ")",
};

inline static QStringList Sequences{
    "DROP TABLE IF EXISTS Sequences",
    "CREATE TABLE Sequences ("
    "id_sequence TEXT NOT NULL COLLATE NOCASE, "
    "sequence_seq TEXT NOT NULL COLLATE NOCASE, "
    //    "sequence_masking TEXT NOT NULL COLLATE NOCASE, "
    "sequence_length INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_sequence), "
    ""
    "CONSTRAINT typeof_length CHECK(TYPEOF(sequence_length) = 'integer'),"
    "CONSTRAINT value_length CHECK(sequence_length > 0) "
    ""
    //    "CONSTRAINT values_masking "
    //    "CHECK(sequence_masking IN ('hard', 'soft', 'none')), "
    //    "CONSTRAINT length_seq CHECK(LENGTH(sequence_seq)), "
    //    ""
    //    "CONSTRAINT fk_Features_seqid "
    //    "FOREIGN KEY (id_sequence) "
    //    "REFERENCES SeqIDs (seqid_name) "
    //    "ON DELETE CASCADE"
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
    "REFERENCES GenomicFeatures (id_feature)"
    ")",

    "CREATE INDEX MGIMap_id_feature ON MGIMap(id_feature)",
};

inline static QStringList GeneOntology{
    "DROP VIEW IF EXISTS GeneOntologySlaves;",
    "DROP TABLE IF EXISTS GeneOntologyAlternativeIDs;",
    "DROP TABLE IF EXISTS GeneOntologyHierarchy;",
    "DROP TABLE IF EXISTS GeneOntologyTerms;",

    "CREATE TABLE GeneOntologyTerms ("
    "id_go INTEGER PRIMARY KEY NOT NULL, "
    "go_name TEXT NOT NULL COLLATE NOCASE, "
    "go_namespace TEXT NOT NULL COLLATE NOCASE, "
    "go_def TEXT NOT NULL COLLATE NOCASE, "
    "go_comment TEXT DEFAULT NULL COLLATE NOCASE, "
    "go_is_obsolete BOOLEAN NOT NULL DEFAULT FALSE "
    ")",

    "CREATE INDEX idx_GeneOntologyTerms_namespace ON "
    "GeneOntologyTerms(go_namespace)",
    "CREATE INDEX idx_GeneOntologyTerms_obsolete ON "
    "GeneOntologyTerms(go_is_obsolete)",

    "CREATE TABLE GeneOntologyHierarchy ("
    "id_go INTEGER NOT NULL,"
    "go_is_a INTEGER DEFAULT NULL,"
    ""
    "PRIMARY KEY (id_go, go_is_a),"
    ""
    "CONSTRAINT fk_id_go "
    "FOREIGN KEY (id_go) "
    "REFERENCES GeneOntologyTerms(id_go) "
    "ON DELETE CASCADE,"
    ""
    "CONSTRAINT fk_go_is_a "
    "FOREIGN KEY (go_is_a) "
    "REFERENCES GeneOntologyTerms(id_go) "
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX idx_GOHierarchy_go_is_a ON GeneOntologyHierarchy(go_is_a)",

    "CREATE TABLE GeneOntologyAlternativeIDs ("
    "go_alt_id INTEGER PRIMARY KEY NOT NULL,"
    "id_go INTEGER NOT NULL,"
    ""
    "CONSTRAINT fk_id_go "
    "FOREIGN KEY (id_go) "
    "REFERENCES GeneOntologyTerms(id_go) "
    "ON DELETE CASCADE"
    ")",

    "CREATE VIEW GeneOntologySlaves AS "
    "SELECT "
    "HM.id_go AS id_go, "
    "HS.id_go AS go_id_slave "
    "FROM GeneOntologyTerms HM CROSS JOIN GeneOntologyTerms HS "
    "WHERE HS.id_go IN"
    "(WITH RECURSIVE GeneOntologySlave(go_slave) AS ("
    "VALUES(HM.id_go) "
    "UNION ALL "
    "SELECT id_go FROM GeneOntologyHierarchy, GeneOntologySlave "
    "WHERE go_is_a = GeneOntologySLave.go_slave "
    ") SELECT * FROM GeneOntologySlave)",
};

inline static QStringList GeneOntologyAnnotation{
    "DROP TABLE IF EXISTS GeneOntologyAnnotation;",
    "CREATE TABLE GeneOntologyAnnotation("
    "id_feature INTEGER NOT NULL, "
    "id_go INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_feature, id_go), "
    ""
    "CONSTRAINT fk_GeneOntologyAnnotation_id_feature "
    "FOREIGN KEY (id_feature) "
    "REFERENCES GenomicFeatures(id_feature) "
    "ON DELETE CASCADE, "
    ""
    "CONSTRAINT fk_GeneOntologyAnnotation_id_go "
    "FOREIGN KEY (id_go) "
    "REFERENCES GeneOntologyTerms(id_go) "
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX idx_GeneOntologyAnnotation_id_go ON "
    "GeneOntologyAnnotation(id_go)",
};

inline static QStringList StructuralVariants{
    "DROP TABLE IF EXISTS StructuralVariants",

    "CREATE TABLE StructuralVariants("
    "id_struct INTEGER PRIMARY KEY NOT NULL,"
    "struct_seqid TEXT NOT NULL COLLATE NOCASE,"
    "struct_source TEXT COLLATE NOCASE,"
    "struct_type TEXT NOT NULL COLLATE NOCASE,"
    "struct_start INT NOT NULL,"
    "struct_end INT NOT NULL,"
    "struct_length INTEGER NOT NULL,"
    "struct_strand COLLATE NOCASE,"
    "struct_signature TEXT NOT NULL COLLATE NOCASE,"
    "struct_study TEXT NOT NULL COLLATE NOCASE,"
    "struct_id INTETEGER NOT NULL,"
    "struct_start_range_start INTEGER DEFAULT NULL,"
    "struct_start_range_end INTEGER DEFAULT NULL,"
    "struct_end_range_start INTEGER DEFAULT NULL,"
    "struct_end_range_end INTEGER DEFAULT NULL,"
    ""
    //    "CONSTRAINT unique_signature UNIQUE (struct_signature),"
    ""
    //    "CONSTRAINT typeof_id_parent "
    //    "CHECK(struct_id_parent IS NULL OR TYPEOF(struct_id_parent) = "
    //    "'integer'),"
    "CONSTRAINT typeof_start "
    "CHECK(TYPEOF(struct_start) = 'integer'),"
    "CONSTRAINT typeof_end "
    "CHECK(TYPEOF(struct_end) = 'integer'),"
    "CONSTRAINT typeof_length "
    "CHECK(TYPEOF(struct_length) = 'integer'),"
    ""
    "CONSTRAINT length_type CHECK(LENGTH(struct_type)),"
    "CONSTRAINT length_signature "
    "CHECK(struct_signature IS NULL OR LENGTH(struct_signature)),"

    "CONSTRAINT value_start CHECK(struct_start > 0),"
    "CONSTRAINT value_end CHECK(struct_end >= struct_start),"
    "CONSTRAINT value_strand "
    "CHECK(struct_strand IS NULL OR struct_strand IN ('+', '-')),"
    ""
    "CONSTRAINT fk_StructuralVariants_seqid "
    "FOREIGN KEY (struct_seqid) "
    "REFERENCES SeqIDs (seqid_name) "
    "ON DELETE CASCADE "
    ""
    //    "CONSTRAINT fk_StructuralVariants_id_parent "
    //    "FOREIGN KEY (struct_id_parent) "
    //    "REFERENCES StructuralVariants (id_struct) "
    //    "ON DELETE CASCADE "
    //    ""
    //    "CONSTRAINT fk_StructuralVariants_parent_signature "
    //    "FOREIGN KEY (struct_parent_signature) "
    //    "REFERENCES StructuralVariants (struct_signature) "
    //    "ON DELETE CASCADE"
    //    ""
    ")",

    "CREATE INDEX idx_StructuralVariantsPosition ON "
    "StructuralVariants(struct_seqid, struct_start, struct_end)",

    "CREATE INDEX idx_StructuralVariantsType ON "
    "StructuralVariants(struct_type)",

    "CREATE INDEX idx_StructuralVariantsLength ON "
    "StructuralVariants(struct_length)",

    "CREATE INDEX idx_StructuralVariantsSignature ON "
    "StructuralVariants(struct_signature)",

    "CREATE INDEX idx_StructuralVariantsStudy ON "
    "StructuralVariants(struct_study)",

    "CREATE INDEX idxStructuralVariantsTypeAndSignature ON "
    "StructuralVariants (struct_type, struct_signature)",

    "CREATE INDEX idxStructuralVariantsTypeAndLength ON "
    "StructuralVariants (struct_type, struct_length)",

    "CREATE INDEX idxStructuralVariantsTypeAndLoc ON "
    "StructuralVariants (struct_type, struct_seqid, struct_start, "
    "struct_end)",

    "DROP TABLE IF EXISTS StructuralVariantsChildren",

    "CREATE TABLE StructuralVariantsChildren("
    "id_struct_child INTEGER PRIMARY KEY NOT NULL,"
    "struct_child_seqid TEXT NOT NULL COLLATE NOCASE,"
    "struct_child_source TEXT COLLATE NOCASE,"
    "struct_child_type TEXT NOT NULL COLLATE NOCASE,"
    "struct_child_start INT NOT NULL,"
    "struct_child_end INT NOT NULL,"
    "struct_child_length INTEGER NOT NULL,"
    "struct_child_strand COLLATE NOCASE,"
    //    "struct_child_id_parent INTEGER DEFAULT NULL,"
    "struct_child_signature TEXT NOT NULL COLLATE NOCASE,"
    "struct_child_study TEXT NOT NULL COLLATE NOCASE,"
    "struct_child_parent_signature TEXT NOT NULL COLLATE NOCASE,"
    "struct_child_id INTETEGER NOT NULL,"
    "struct_child_start_range_start INTEGER DEFAULT NULL,"
    "struct_child_start_range_end INTEGER DEFAULT NULL,"
    "struct_child_end_range_start INTEGER DEFAULT NULL,"
    "struct_child_end_range_end INTEGER DEFAULT NULL,"
    ""
    //    "CONSTRAINT unique_signature UNIQUE (struct_signature),"
    ""
    //    "CONSTRAINT typeof_id_parent "
    //    "CHECK(TYPEOF(struct_child_id_parent) = 'integer'),"
    "CONSTRAINT typeof_start "
    "CHECK(TYPEOF(struct_child_start) = 'integer'),"
    "CONSTRAINT typeof_end "
    "CHECK(TYPEOF(struct_child_end) = 'integer'),"
    "CONSTRAINT typeof_length "
    "CHECK(TYPEOF(struct_child_length) = 'integer'),"
    ""
    "CONSTRAINT length_type CHECK(LENGTH(struct_child_type)),"
    "CONSTRAINT length_signature CHECK(LENGTH(struct_child_signature)),"

    "CONSTRAINT value_start CHECK(struct_child_start > 0),"
    "CONSTRAINT value_end CHECK(struct_child_end >= struct_child_start),"
    "CONSTRAINT value_strand "
    "CHECK(struct_child_strand IS NULL OR struct_child_strand IN ('+', '-')),"
    ""
    "CONSTRAINT fk_StructuralVariants_seqid "
    "FOREIGN KEY (struct_child_seqid) "
    "REFERENCES SeqIDs (seqid_name) "
    "ON DELETE CASCADE "
    ""
    //    "CONSTRAINT fk_StructuralVariants_id_parent "
    //    "FOREIGN KEY (struct_child_id_parent) "
    //    "REFERENCES StructuralVariants (id_struct) "
    //    "ON DELETE CASCADE "
    //    ""
    //    "CONSTRAINT fk_StructuralVariants_parent_signature "
    //    "FOREIGN KEY (struct_parent_signature) "
    //    "REFERENCES StructuralVariants (struct_signature) "
    //    "ON DELETE CASCADE"
    //    ""
    ")",

    "CREATE INDEX idx_StructuralVariantsChildrenPosition ON "
    "StructuralVariantsChildren(struct_child_seqid, struct_child_start, "
    "struct_child_end)",

    "CREATE INDEX idx_StructuralVariantsChildrenType ON "
    "StructuralVariantsChildren(struct_child_type)",

    "CREATE INDEX idx_StructuralVariantsChildrenLength ON "
    "StructuralVariantsChildren(struct_child_length)",

    "CREATE INDEX idx_StructuralVariantsChildrenSignature ON "
    "StructuralVariantsChildren(struct_child_signature)",

    "CREATE INDEX idx_StructuralVariantsParentChildrenSignature ON "
    "StructuralVariantsChildren(struct_child_parent_signature)",

    "CREATE INDEX idx_StructuralVariantsChildrenStudy ON "
    "StructuralVariantsChildren(struct_child_study)",

    "CREATE INDEX idxStructuralVariantsChildrenTypeAndLength ON "
    "StructuralVariantsChildren(struct_child_type, struct_child_length)",

    "CREATE INDEX idxStructuralVariantsChildrenTypeAndSignature ON "
    "StructuralVariantsChildren(struct_child_type, struct_child_signature)",

    "CREATE INDEX idx_StructuralVariantsChildrenTypeParentSignature ON "
    "StructuralVariantsChildren(struct_child_type, "
    "struct_child_parent_signature)",

    "CREATE INDEX idxStructuralVariantsChildrenTypeAndLoc ON "
    "StructuralVariantsChildren(struct_child_type, struct_child_seqid, "
    "struct_child_start, struct_child_end)",

}; // namespace Schemas

} // namespace Schemas

using namespace Execute;

void init_main(QSqlDatabase &db);
void init_taxon(QSqlDatabase &db, QString taxons = {});
void init_genomic_features(QSqlDatabase &db);
void init_genomic_sequences(QSqlDatabase &db);
void init_map_uniprot(QSqlDatabase &db);
void init_map_mgi(QSqlDatabase &db);
void init_gene_ontology(QSqlDatabase &db);
void init_structural_variants(QSqlDatabase &db);
} // namespace DOGIToys::Initiate
