#include <dogitoys/populate/gene_ontology.hpp>

using namespace DOGIToys;
using namespace DOGIToys::GeneOntology;

void GeneOntologyTerm::parse(const QString &line) {
  const auto [field, value] = get_field_value(line, ':', 1);

  if (field == "id")
    setID(value);
  else if (field == "name")
    setName(value);
  else if (field == "namespace")
    setNamespace(value);
  else if (field == "def")
    setDef(value);
  else if (field == "synonym")
    addSynonym(value);
  else if (field == "is_a")
    addIsA(value);
  else if (field == "is_obsolete")
    setIsObsolete(value);
  else if (field == "comment")
    setComment(value);
  else if (field == "consider")
    addConsider(value);
  else if (field == "relationship")
    addRelationship(value);
  else if (field == "intersection_of")
    addIntersectionOf(value);
  else if (field == "replaced_by")
    setReplacedBy(value);
  else if (field == "alt_id")
    addAltID(value);
  else if (field == "subset")
    return;
  else if (field == "xref")
    return;
  else if (field == "disjoint_from")
    return;
  else if (field == "created_by")
    return;
  else if (field == "creation_date")
    return;
  else if (field == "property_value")
    return;
  else
    throw_runerror("Unsupported field: " + line);
}

void GeneOntologyTerm::setID(const QString &id) {
  if (const auto temp = extractIntID(id, ':'); this->id != 0)
    throw_runerror("ID already initialized");
  else {
    this->id = temp;
    //    this->alt_id.append(temp);
  }
}

void GeneOntologyTerm::setName(const QString &name) {
  if (!this->name.isEmpty()) throw_runerror("Name already initialized");
  this->name = name;
}

void GeneOntologyTerm::setNamespace(const QString &names) {
  if (!this->names.isEmpty()) throw_runerror("Namespace already initialized");
  this->names = names;
}

void GeneOntologyTerm::setDef(const QString &def) {
  if (!this->def.isEmpty()) throw_runerror("Def already initialized");
  this->def = def;
}

void GeneOntologyTerm::addSynonym(const QString &synonym) {
  this->synonyms.append(synonym);
}

void GeneOntologyTerm::addIsA(const QString &is_a) {
  this->is_a.append(extractIntID(is_a, ':'));
}

void GeneOntologyTerm::setIsObsolete(const QString &obsolete) {
  this->is_obsolete = obsolete == "true" ? true : false;
}

void GeneOntologyTerm::setComment(const QString &comment) {
  if (!this->comment.isEmpty()) throw_runerror("Comment already initialized");

  this->comment = comment;
}

void GeneOntologyTerm::addConsider(const QString &consider) {
  this->consider.append(extractIntID(consider, ':'));
}

void GeneOntologyTerm::addIntersectionOf(const QString &intersection_of) {
  this->intersection_of.append(extractIntID(intersection_of, ':'));
}

void GeneOntologyTerm::addRelationship(const QString &relationship) {
  this->relationship.append(extractIntID(relationship, ':'));
}

void GeneOntologyTerm::setReplacedBy(const QString &replaced_by) {
  if (this->replaced_by != 0) throw_runerror("ID already initialized");
  this->replaced_by = extractIntID(replaced_by, ':');
}

void GeneOntologyTerm::addAltID(const QString &alt_id) {
  this->alt_id.append(extractIntID(alt_id, ':'));
}

std::optional<GeneOntologyTerm> OBOParser::getTerm() {
  if (!reader.setLineToMatch("[Term]")) return nullopt;

  auto term = GeneOntologyTerm();

  while (auto line = reader()) {
    if ((*line).isEmpty()) break;
    term.parse(*line);
  }

  return term;
}

void GeneOntologyTerm::insert(QSqlDatabase &db) {
  auto insert =
      Execute::prepare(db,
                       "INSERT INTO GeneOntologyTerms "
                       "(id_go, go_name, go_namespace, "
                       "go_def, go_comment, go_is_obsolete) "
                       "VALUES (:id_go, :go_name, :go_namespace, :go_def, "
                       ":go_comment, :go_is_obsolete)");

  insert.bindValue(":id_go", id);
  insert.bindValue(":go_name", name);
  insert.bindValue(":go_namespace", names);
  insert.bindValue(":go_def", def);

  if (comment.isEmpty())
    insert.bindValue(":go_comment", QVariant(QVariant::String));
  else
    insert.bindValue(":go_comment", comment);

  insert.bindValue(":go_is_obsolete", is_obsolete);

  Execute::exec(insert);

  insert = Execute::prepare(db,
                            "INSERT INTO GeneOntologyAlternativeIDs "
                            "(go_alt_id, id_go) "
                            "VALUES (:go_alt_id, :id_go)");

  insert.bindValue(":id_go", id);

  for (const auto &go_alt_id : getAltID()) {
    insert.bindValue(":go_alt_id", go_alt_id);
    Execute::exec(insert);
  }
}

void GeneOntology::insert_go_hierarchy(const QSqlDatabase &db, int id_go,
                                       int go_is_a) {
  auto insert = Execute::prepare(db,
                                 "INSERT INTO GeneOntologyHierarchy "
                                 "(id_go, go_is_a) "
                                 "VALUES (:id_go, :go_is_a)");

  insert.bindValue(":id_go", id_go);
  insert.bindValue(":go_is_a", go_is_a ? go_is_a : QVariant(QVariant::Int));

  Execute::exec(insert);
}

GAFRecord::GAFRecord(const QString &line, bool from_mgi) : from_mgi{from_mgi} {
  const auto &data = line.split("\t");

  uniprot_xref = data[1];
  id_go = extractIntID(data[4], ':');
}

void GAFRecord::insert(QSqlDatabase &db) const {
  auto insert = Execute::prepare(db,
                                 "INSERT OR IGNORE INTO "
                                 "GeneOntologyAnnotation (id_feature, id_go) "
                                 "VALUES (:id_feature, :id_go)");
  insert.bindValue(":id_go", id_go);

  if (from_mgi) {
    if (const auto id_feature = Select::select_id_feature_from_mgi(
            db, extractIntID(uniprot_xref, ':'));
        id_feature) {
      insert.bindValue(":id_feature", id_feature);
      Execute::exec(insert);
    }
  } else {
    for (const int id_feature :
         Select::select_id_feature_from_uniprot(db, uniprot_xref)) {
      insert.bindValue(":id_feature", id_feature);
    }
  }
}
