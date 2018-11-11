#include "dogitoys-gff.h"

using namespace DOGIToys;
using namespace DOGIGFF;

using DOGITools::exec;
using DOGITools::prepare;
using DOGITools::throw_error;

GFFRecord::GFFRecord(const QString &line) { parse(line); }


GVFStructural::GVFStructural(const QString line) : GFFRecord(line) {
  study = attributes.value("study_accession");
  signature = DOGITools::extractID(attributes.value("Dbxref"));
}

void GVFStructural::insert(const QSqlDatabase &db, const QString &id_database,
                           int id_feature) {
  QSqlQuery insert(db);

  prepare(insert, this->hasParent() ? queryChild : queryParent);

  GFFRecord::bind(insert, id_database, id_feature);

  insert.bindValue(":signature_parent",
                   parent.isEmpty() ? QVariant(QVariant::String) : parent);
  insert.bindValue(":study", study);

  exec(insert);
}

void GVFStructural::updateParent(const QSqlDatabase &db,
                                 const IDFeature &parent,
                                 const IDFeature &child) {

  if (DOGIFetch::selectIDParent(db, "VarStructuralChildren", child.id_database,
                                child.id_feature))
    throw_error(child.toQString() + " already has a parent");

  QSqlQuery update(db);

  prepare(update, "UPDATE VarStructuralChildren "
                  "SET feature_id_parent = :id_parent "
                  "WHERE id_database = :id_database "
                  "AND id_feature = :id_feature");

  update.bindValue(":id_parent", parent.id_feature);
  update.bindValue(":id_database", child.id_database);
  update.bindValue(":id_feature", child.id_feature);

  exec(update);
}

GFFRegulation::GFFRegulation(const QString line) : GFFRecord(line) {
  signature = attributes.value("ID");
  description = attributes.value("feature_description");
  type = attributes.value("feature_type");
}

void GFFRegulation::insert(const QSqlDatabase &db, const QString &id_database,
                           int id_feature) {
  QSqlQuery insert(db);

  prepare(insert,
          "INSERT INTO RegulatoryFeatures (" + gff_fields_basic.join(", ") +
              ", feature_signature, feature_description) "
              "VALUES (:" +
              gff_fields_basic.join(", :") + ", :signature, :description)");

  GFFRecord::bind(insert, id_database, id_feature);

  insert.bindValue(":signature", signature);
  insert.bindValue(":description", description);

  exec(insert);
}
