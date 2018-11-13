inline static QStringList GeneMapping{
    "DROP TABLE IF EXISTS MapUniprot",
    "DROP TABLE IF EXISTS MapGFF3",
    "DROP TABLE IF EXISTS MapMGI",

    "CREATE TABLE MapUniprot ("
    "id_uniprot TEXT NOT NULL COLLATE NOCASE,"
    "id_database TEXT NOT NULL COLLATE NOCASE, "
    "id_feature INTEGER NOT NULL, "

    "uniprot_source TEXT NOT NULL COLLATE NOCASE, "
    "uniprot_type TEXT NOT NULL COLLATE NOCASE, "
    ""
    "PRIMARY KEY (id_uniprot, id_database,id_feature),"
    ""
    "CONSTRAINT fk_MapUniprot_feature "
    "FOREIGN KEY (id_database, id_feature) "
    "REFERENCES GFF3Features (id_database, id_feature)"
    ")",

    "CREATE INDEX idx_MapUniprot_uniprot_source ON MapUniprot(uniprot_source)",
    "CREATE INDEX idx_MapUniprot_uniprot_type ON MapUniprot(uniprot_type)",

    "CREATE TABLE MapGFF3 ("
    "id_mapping INTEGER PRIMARY KEY NOT NULL,"
    "id_database_from TEXT NOT NULL COLLATE NOCASE,"
    "id_feature_from INTEGER NOT NULL,"
    "id_database_to TEXT NOT NULL COLLATE NOCASE,"
    "id_feature_to INTEGER NOT NULL,"
    ""
    "CONSTRAINT unique_mapping "
    "UNIQUE(id_database_from, id_feature_from, id_database_to, id_feature_to),"
    ""
    "CONSTRAINT fk_MapGFF3_from "
    "FOREIGN KEY (id_database_from, id_feature_from) "
    "REFERENCES GFF3Features(id_database, id_feature),"
    ""
    "CONSTRAINT fk_MapGFF3_to "
    "FOREIGN KEY (id_database_to, id_feature_to) "
    "REFERENCES GFF3Features(id_database, id_feature)"
    ")",
    "CREATE INDEX idx_MapGFF3_from ON MapGFF3(id_database_from, "
    "id_feature_from)",
    "CREATE INDEX idx_MapGFF3_to ON MapGFF3(id_database_to, id_feature_to)",

    "CREATE TABLE MapMGI ("
    "id_mgi INTEGER NOT NULL COLLATE NOCASE,"
    "id_database TEXT NOT NULL COLLATE NOCASE, "
    "id_feature INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_mgi, id_database,id_feature),"
    ""
    "CONSTRAINT fk_MapMGI_feature "
    "FOREIGN KEY (id_database, id_feature) "
    "REFERENCES GFF3Features (id_database, id_feature)"
    ")",

    "CREATE INDEX MapMGI_id_feature ON MapMGI(id_database, id_feature)"};

inline static QStringList HPA_TissueExpression{
    "DROP TABLE IF EXISTS HPAGenesExpression",
    "DROP TABLE IF EXISTS HPATissues",

    "CREATE TABLE HPATissues ("
    "id_tissue TEXT PRIMARY KEY NOT NULL COLLATE NOCASE,"
    ""
    "CONSTRAINT length_id_tissue CHECK(LENGTH(id_tissue))"
    ")",

    "CREATE TABLE HPAGenesExpression ("
    "id_database TEXT NOT NULL COLLATE NOCASE,"
    "id_feature INTEGER NOT NULL,"
    "id_tissue TEXT NOT NULL COLLATE NOCASE, "
    "expression_tpm REAL NOT NULL,"
    ""
    "PRIMARY KEY (id_database, id_feature, id_tissue),"
    ""
    "CONSTRAINT expression_type CHECK(TYPEOF(expression_tpm) = 'real'),"
    ""
    "CONSTRAINT value_expression_tpm CHECK(expression_tpm >= 0.0),"
    ""
    "CONSTRAINT fk_GenesExpression_id_feature "
    "FOREIGN KEY (id_database, id_feature) "
    "REFERENCES GFF3Features(id_database, id_feature)"
    "ON DELETE CASCADE,"
    ""
    "CONSTRAINT fk_tissue "
    "FOREIGN KEY (id_tissue) "
    "REFERENCES HPATissues(id_tissue)"
    "ON DELETE CASCADE"
    ")",

    "CREATE INDEX fk_tissue ON HPAGenesExpression(id_tissue)"};

inline static QStringList GeneOntology{
    "DROP VIEW IF EXISTS GOSlaves;",
    "DROP TABLE IF EXISTS GOAnnotations;",
    "DROP TABLE IF EXISTS GOHierarchy;",
    "DROP TABLE IF EXISTS GOTerms;",

    "CREATE TABLE GOTerms ("
    "id_go INTEGER PRIMARY KEY NOT NULL, "
    "go_name TEXT NOT NULL COLLATE NOCASE, "
    "go_namespace TEXT NOT NULL COLLATE NOCASE, "
    "go_def TEXT NOT NULL COLLATE NOCASE, "
    "go_comment TEXT DEFAULT NULL COLLATE NOCASE, "
    "go_is_obsolete BOOLEAN NOT NULL DEFAULT FALSE, "
    "go_id_master INTEGER DEFAULT NULL, "
    ""
    "CONSTRAINT fk_GOTerms_id_master "
    "FOREIGN KEY (go_id_master) "
    "REFERENCES GOTerms(id_go) "
    ")",

    "CREATE INDEX idx_GOTerms_namespace ON GOTerms(go_namespace)",

    "CREATE TABLE GOHierarchy ("
    "id_go INTEGER NOT NULL,"
    "go_is_a INTEGER DEFAULT NULL,"
    ""
    "PRIMARY KEY (id_go, go_is_a),"
    ""
    "CONSTRAINT fk_id_go "
    "FOREIGN KEY (id_go) "
    "REFERENCES GOTerms(id_go)"
    "ON DELETE CASCADE,"
    ""
    "CONSTRAINT fk_go_is_a "
    "FOREIGN KEY (go_is_a) "
    "REFERENCES GOTerms(id_go)"
    "ON DELETE CASCADE"
    ""
    ")",

    "CREATE INDEX idx_GOHierarchy_go_is_a ON GOHierarchy(go_is_a)",

    "CREATE TABLE GOAnnotations("
    "id_database TEXT NOT NULL COLLATE NOCASE, "
    "id_feature INTEGER NOT NULL, "
    "id_go INTEGER NOT NULL, "
    ""
    "PRIMARY KEY (id_database, id_feature, id_go), "
    ""
    "CONSTRAINT fk_GOAnnotations_id_feature "
    "FOREIGN KEY (id_database, id_feature) "
    "REFERENCES GFF3Features(id_database, id_feature) "
    "ON DELETE CASCADE,"
    ""
    "CONSTRAINT fk_GOAnnotations_id_go "
    "FOREIGN KEY (id_go) "
    "REFERENCES GOTerms(id_go) "
    "ON DELETE CASCADE"
    ")",

    "CREATE VIEW GOSlaves AS "
    "SELECT "
    "HM.id_go AS id_go, "
    "HS.id_go AS go_id_slave "
    "FROM GOTerms HM CROSS JOIN GOTerms HS "
    "WHERE HS.id_go IN"
    "(WITH RECURSIVE GOSlave(go_slave) AS ("
    "VALUES(HM.id_go) "
    "UNION ALL "
    "SELECT id_go FROM GOHierarchy, GOSlave "
    "WHERE go_is_a = GOSLave.go_slave "
    ") SELECT * FROM GOSlave)"};

inline static QStringList Variants{
    "DROP TABLE IF EXISTS VarStructural",
    "DROP TABLE IF EXISTS VarStructuralChildren",
    "CREATE TABLE VarStructural(" +
        QString(GFFBasic).remove("feature_id_parent INTEGER,") +
        "feature_study TEXT NOT NULL COLLATE NOCASE,"
        ""
        "PRIMARY KEY (id_database, id_feature), "
        ""
        "CONSTRAINT fk_GVFFeatures_seqid "
        "FOREIGN KEY (id_database, feature_seqid) "
        "REFERENCES GFF3SeqIDs (id_database, seqid_name)"
        ")",

    "CREATE INDEX VarStructural_type ON "
    "VarStructural(feature_type, id_database);",

    "CREATE INDEX VarStructural_signature ON "
    "VarStructural(feature_signature, id_database);",

    "CREATE INDEX VarStructural_study ON "
    "VarStructural(feature_study, id_database);",

    "CREATE INDEX VarStructural_seqid ON "
    "VarStructural(feature_seqid, id_database);",

    "CREATE TABLE VarStructuralChildren(" + GFFBasic +
        "feature_signature_parent TEXT NOT NULL COLLATE NOCASE,"
        "feature_study TEXT NOT NULL COLLATE NOCASE,"
        ""
        "PRIMARY KEY (id_database, id_feature), "
        ""
        "CONSTRAINT fk_VarStructuralChildren_seqid "
        "FOREIGN KEY (id_database, feature_seqid) "
        "REFERENCES GFF3SeqIDs (id_database, seqid_name),"
        ""
        "CONSTRAINT fk_VarStructuralChildren_id_parent "
        "FOREIGN KEY (id_database, feature_id_parent) "
        "REFERENCES VarStructural (id_database, id_feature)"
        ")",

    "CREATE INDEX VarStructuralChildren_type ON "
    "VarStructural(id_database, feature_type);",

    "CREATE INDEX VarStructuralChildren_signature ON "
    "VarStructuralChildren(id_database, feature_signature);",

    "CREATE INDEX VarStructuralChildren_study ON "
    "VarStructuralChildren(id_database, feature_study);",

    "CREATE INDEX VarStructuralChildren_seqid ON "
    "VarStructuralChildren(id_database, feature_seqid);",

    "CREATE INDEX VarStructuralChildren_id_parent ON "
    "VarStructuralChildren(id_database, feature_id_parent);",

    "CREATE INDEX VarStructuralChildren_signature_parent ON "
    "VarStructuralChildren(id_database, feature_signature_parent);",
};

// inline static QStringList GVF{
//    "DROP TABLE IF EXISTS GVFFeatures",
//    "CREATE TABLE GVFFeatures(" + GFFBasic +
//        "feature_parent_signature TEXT COLLATE NOCASE,"
//        "feature_study TEXT NOT NULL COLLATE NOCASE,"
//        ""
//        "PRIMARY KEY (id_database, id_feature), "
//        ""
//        "CONSTRAINT fk_GVFFeatures_seqid "
//        "FOREIGN KEY (id_database, feature_seqid) "
//        "REFERENCES GFF3SeqIDs (id_database, seqid_name),"
//        ""
//        "CONSTRAINT fk_GVFFeatures_id_parent "
//        "FOREIGN KEY (id_database, feature_id_parent) "
//        "REFERENCES GVFFeatures (id_database, id_feature)"
//        ")",

//    "CREATE INDEX GVFFeature_type ON GVFFeatures(feature_type);",
//    "CREATE INDEX GVFFeature_parent ON "
//    "GVFFeatures(id_database, feature_id_parent);",
//    "CREATE INDEX GVFFeature_signature ON GVFFeatures(feature_signature);",
//    "CREATE INDEX GVFFeature_parent_signature ON "
//    "GVFFeatures(feature_parent_signature);",
//    "CREATE INDEX GVFFeature_study ON GVFFeatures(feature_study);",
//    "CREATE INDEX GVFFeature_seqid ON GVFFeatures(feature_seqid);",
//};

inline static QStringList RegulatoryFeatures{
    "DROP TABLE IF EXISTS RegulatoryFeatures",
    "CREATE TABLE RegulatoryFeatures(" + GFFBasic +
        ""
        "feature_description TEXT COLLATE NOCASE,"
        ""
        "PRIMARY KEY (id_database, id_feature), "
        ""
        "CONSTRAINT fk_RegulatoryFeatures_seqid "
        "FOREIGN KEY (id_database, feature_seqid) "
        "REFERENCES GFF3SeqIDs (id_database, seqid_name),"
        ""
        "CONSTRAINT fk_RegulatoryFeatures_id_parent "
        "FOREIGN KEY (id_database, feature_id_parent) "
        "REFERENCES RegulatoryFeatures (id_database, id_feature)"
        ""
        ")",

    "CREATE INDEX idx_RegulatoryFeature_type ON "
    "RegulatoryFeatures(feature_type);",
    "CREATE UNIQUE INDEX idx_RegulatoryFeatures_signature "
    "ON RegulatoryFeatures(feature_signature)",
    "CREATE INDEX idx_RegulatoryFeatures_id_parent "
    "ON RegulatoryFeatures(id_database, feature_id_parent)",
};

}  // namespace DOGIToys