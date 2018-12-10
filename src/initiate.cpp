#include "dogitoys/initiate.hpp"

void DOGIToys::Initiate::init_main(QSqlDatabase &db) {
  qInfo() << "Initiating Main Tables";
  Execute::exec(db, Schemas::DOGI_main);
}

void DOGIToys::Initiate::init_taxon(QSqlDatabase &db, QString taxons) {
  qInfo() << "Initiating Taxons";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::Taxons);

  if (taxons.isEmpty()) {
    auto insert = prepare(db,
                          "INSERT INTO Taxons (id_taxon, taxon_name) "
                          "VALUES (:id_taxon, :taxon_name)");

    for (const auto &[id_taxon, taxon_name] : Schemas::BasicTaxonIDs) {
      insert.bindValue(":id_taxon", id_taxon);
      insert.bindValue(":taxon_name", taxon_name);
      exec(insert);
      if (taxon_name.contains(' ')) {
        auto insert_alias =
            prepare(db,
                    "INSERT INTO TaxonAliases (id_alias, id_taxon) "
                    "VALUES (:id_alias, :id_taxon)");
        insert_alias.bindValue(":id_alias",
                               QString(taxon_name).replace(' ', '_'));
        insert_alias.bindValue(":id_taxon", id_taxon);
        exec(insert_alias);
      }
      insert.finish();
    }

    prepare(insert,
            "INSERT INTO TaxonAliases (id_alias, id_taxon) "
            "VALUES (:id_alias, :id_taxon)");

    for (const auto &[id_alias, id_taxon] : Schemas::BasicTaxonAliases) {
      insert.bindValue(":id_alias", id_alias);
      insert.bindValue(":id_taxon", id_taxon);
      exec(insert);
      if (id_alias.contains(' ')) {
        qInfo() << QString(id_alias).replace(' ', '_');
        insert.bindValue(":id_alias", QString(id_alias).replace(' ', '_'));
        exec(insert);
      }

      insert.finish();
    }

    Transaction::commit(db);

  } else {
    Transaction::rollback(db);
    throw_runerror("Loading taxons from external file is not supported!!!");
  }
}

void DOGIToys::Initiate::init_genomic_features(QSqlDatabase &db) {
  qInfo() << "Initiating GenomicFeatures";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::SeqIDs);
  Execute::exec(db, Schemas::GenomicFeatures);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_genomic_sequences(QSqlDatabase &db) {
  qInfo() << "Initiating GenomicSequences";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::GenomicSequences);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_map_uniprot(QSqlDatabase &db) {
  qInfo() << "Initiating UniprotMap";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::UniprotMap);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_gene_ontology(QSqlDatabase &db) {
  qInfo() << "Initiating GeneOntology";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::GeneOntology);
  Execute::exec(db, Schemas::GeneOntologyAnnotation);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_map_mgi(QSqlDatabase &db) {
  Transaction::transaction(db);
  Execute::exec(db, Schemas::MGIMap);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_structural_variants(QSqlDatabase &db) {
  Transaction::transaction(db);
  Execute::exec(db, Schemas::StructuralVariants);
  Transaction::commit(db);
}
