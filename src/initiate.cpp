#include "dogitoys/initiate.hpp"

void DOGIToys::Initiate::init_main(QSqlDatabase db) {
  qInfo() << "Initiating Main Tables";
  Execute::exec(db, Schemas::DOGI_main);
}

void DOGIToys::Initiate::init_taxon(QSqlDatabase db, QString taxons) {
  qInfo() << "Initiating Taxons";
  Execute::exec(db, Schemas::Taxons);
  if (taxons.isEmpty()) {
    Transaction::transaction(db);
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
    throw_runerror("Loading taxons from external file is not supported!!!");
  }
}

void DOGIToys::Initiate::init_genomic_features(QSqlDatabase db) {
  qInfo() << "Initiating GenomicFeatures";
  Execute::exec(db, Schemas::SeqIDs);
  Execute::exec(db, Schemas::GenomicFeatures);
}

void DOGIToys::Initiate::init_genomic_sequences(QSqlDatabase db) {
  qInfo() << "Initiating GenomicSequences";
  Execute::exec(db, Schemas::GenomicSequences);
}
