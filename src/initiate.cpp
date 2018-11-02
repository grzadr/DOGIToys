#include "dogitoys/initiate.hpp"

void DOGIToys::Initiate::Initializer::reset() { db.reset(); }

void DOGIToys::Initiate::Initializer::init_main() {
  qWarning() << "Initiating main tables";
  Execute::exec(*db, Schemas::DOGI_main);
  qInfo() << "Adding basic taxons";
  init_taxon();
}

void DOGIToys::Initiate::Initializer::init_taxon() {
  db->transaction();

  QSqlQuery insert(*db);

  prepare(insert,
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

  db->commit();
}
