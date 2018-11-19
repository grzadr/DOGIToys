#include "dogitoys/initiate.hpp"

void DOGIToys::Initiate::init_main(QSqlDatabase db) {
  qInfo() << "Initiating Main Tables";
  Execute::exec(db, Schemas::DOGI_main);
}

void DOGIToys::Initiate::init_taxon(QSqlDatabase db, QString taxons) {
  qInfo() << "Initiating Taxons";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::Taxons);

  if (taxons.isEmpty()) {
    auto insert = prepare(db,
                          "INSERT INTO Taxons (id_taxon, taxon_name) "
                          "VALUES (:id_taxon, :taxon_name)");

    for (const auto& [id_taxon, name] : Schemas::BasicTaxonIDs) {
      insert.bindValue(":id_taxon", id_taxon);
      insert.bindValue(":taxon_name", name);
      exec(insert);
      insert.finish();
    }

    prepare(insert,
            "INSERT INTO TaxonAliases (id_alias, id_taxon) "
            "VALUES (:id_alias, :id_taxon)");

    for (const auto& [id_alias, id_taxon] : Schemas::BasicTaxonAliases) {
      insert.bindValue(":id_alias", id_alias);
      insert.bindValue(":id_taxon", id_taxon);
      exec(insert);
      insert.finish();
    }

    Transaction::commit(db);

  } else {
    Transaction::rollback(db);
    throw_runerror("Loading taxons from external file is not supported!!!");
  }
}

void DOGIToys::Initiate::init_genomic_features(QSqlDatabase db) {
  qInfo() << "Initiating GenomicFeatures";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::SeqIDs);
  Execute::exec(db, Schemas::GenomicFeatures);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_genomic_sequences(QSqlDatabase db) {
  qInfo() << "Initiating GenomicSequences";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::GenomicSequences);
  Transaction::commit(db);
}

void DOGIToys::Initiate::init_uniprot_map(QSqlDatabase db) {
  qInfo() << "Initiating UniprotMap";
  Transaction::transaction(db);
  Execute::exec(db, Schemas::UniprotMap);
  Transaction::commit(db);
}
