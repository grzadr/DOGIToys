#include "dogitoys/initiate.hpp"

void DOGIToys::Initiate::init_main(QSqlDatabase db) {
  qInfo() << "Initiating main tables";
  Execute::exec(db, Schemas::DOGI_main);
}

void DOGIToys::Initiate::init_taxon(QSqlDatabase db, QString taxons) {
  qInfo() << "Initiating taxons";
  if (taxons.isEmpty()) {
    db.transaction();
    auto insert = prepare(db,
                          "INSERT INTO DOGITaxons (id_taxon, taxon_name) "
                          "VALUES (:id_taxon, :taxon_name)");

    for (const auto& [id_taxon, name] : Schemas::Taxons) {
      insert.bindValue(":id_taxon", id_taxon);
      insert.bindValue(":taxon_name", name);
      exec(insert);
      insert.finish();
    }

    prepare(insert,
            "INSERT INTO DOGITaxonAliases (id_alias, id_taxon) "
            "VALUES (:id_alias, :id_taxon)");

    for (const auto& [id_alias, id_taxon] : Schemas::TaxonAliases) {
      insert.bindValue(":id_alias", id_alias);
      insert.bindValue(":id_taxon", id_taxon);
      exec(insert);
      insert.finish();
    }

    db.commit();
  } else {
    throw_runerror("Loading taxons from external file is not supported!!!");
  }
}
