#include <dogitoys/populate/gene_ontology.hpp>

using namespace DOGIToys::GeneOntology;

void GeneOntologyTerm::setID(const QString &id) {
  if (const auto temp = extractID(id); this->id != 0)
    throw_runerror("ID already initialized");
  else {
    this->id = temp;
    this->alt_id.append(temp);
  }
}

void GeneOntologyTerm::setName(const QString &name) {
  if (!this->name.isEmpty()) throw throw_runerror("Name already initialized");
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
  this->is_a.append(extractID(is_a, '!'));
}

void GeneOntologyTerm::setIsObsolete(const QString &obsolete) {
  this->is_obsolete = obsolete == "true" ? true : false;
}

void GeneOntologyTerm::setComment(const QString &comment) {
  if (!this->comment.isEmpty()) throw_runerror("Comment already initialized");

  this->comment = comment;
}

void GeneOntologyTerm::addConsider(const QString &consider) {
  this->consider.append(extractID(consider));
}

void GeneOntologyTerm::addIntersectionOf(const QString &intersection_of) {
  this->intersection_of.append(extractID(intersection_of, '!'));
}

void GeneOntologyTerm::addRelationship(const QString &relationship) {
  this->relationship.append(extractID(relationship, '!'));
}

void GeneOntologyTerm::setReplacedBy(const QString &replaced_by) {
  if (this->replaced_by != 0) throw_runerror("ID already initialized");
  this->replaced_by = extractID(replaced_by);
}

void GeneOntologyTerm::addAltID(const QString &alt_id) {
  this->alt_id.append(extractID(alt_id));
}

std::optional<GeneOntologyTerm> OBOParser::parse() {
  do {
    file->readLineInto();
  } while (file->line != "[Term]");

  auto term = GeneOntologyTerm();

  while (file->readLineInto()) {
    const auto &field = file->line.left(file->line.indexOf(':'));
    const auto &value = file->line.mid(file->line.indexOf(':') + 2);

    if (file->line.isEmpty())
      result.append(term);
    else if (field == "[Term]")
      term = GeneOntologyTerm();
    else if (field == "id")
      term.setID(value);
    else if (field == "name")
      term.setName(value);
    else if (field == "namespace")
      term.setNamespace(value);
    else if (field == "def")
      term.setDef(value);
    else if (field == "synonym")
      term.addSynonym(value);
    else if (field == "is_a")
      term.addIsA(value);
    else if (field == "is_obsolete")
      term.setIsObsolete(value);
    else if (field == "comment")
      term.setComment(value);
    else if (field == "consider")
      term.addConsider(value);
    else if (field == "relationship")
      term.addRelationship(value);
    else if (field == "intersection_of")
      term.addIntersectionOf(value);
    else if (field == "replaced_by")
      term.setReplacedBy(value);
    else if (field == "alt_id")
      term.addAltID(value);
    else if (field == "subset")
      continue;
    else if (field == "xref")
      continue;
    else if (field == "disjoint_from")
      continue;
    else if (field == "created_by")
      continue;
    else if (field == "creation_date")
      continue;
    else if (field == "property_value")
      continue;
    else if (field == "[Typedef]")
      break;
    else
      throw runtime_error(file->line.toStdString());
  }

  return result;
}

void DOGIGOParser::setFile(const QString &file_name) {
  file = make_unique<DOGIFile>(file_name);
}

void DOGIGO::insertGOTerm(const QSqlDatabase &db, int id_go,
                          const QString &go_name, const QString &go_namespace,
                          const QString &go_def, const QString &go_comment,
                          bool go_is_obsolete, int go_id_master) {
  QSqlQuery query(db);

  prepare(query,
          "INSERT INTO "
          "GOTerms (id_go, go_name, go_namespace, go_def, go_comment, "
          "go_is_obsolete, go_id_master) "
          "VALUES(:id_go, :go_name, :go_namespace, :go_def, "
          ":go_comment, :go_is_obsolete, :go_id_master)");

  query.bindValue(":id_go", id_go);
  query.bindValue(":go_name", go_name);
  query.bindValue(":go_namespace", go_namespace);
  query.bindValue(":go_def", go_def);
  if (go_comment.isEmpty())
    query.bindValue(":go_comment", QVariant(QVariant::String));
  else
    query.bindValue(":go_comment", go_comment);
  query.bindValue(":go_is_obsolete", go_is_obsolete);
  query.bindValue(":go_id_master", go_id_master);

  exec(query);
}

void DOGIGO::insertGOHierarchy(const QSqlDatabase &db, int id_go, int go_is_a) {
  QSqlQuery query(db);

  prepare(query,
          "INSERT INTO GOHierarchy (id_go, go_is_a) VALUES (:id_go, :go_is_a)");

  query.bindValue(":id_go", id_go);
  query.bindValue(":go_is_a", go_is_a ? go_is_a : QVariant(QVariant::Int));

  exec(query);
}

qvec_pair_int DOGIGO::insertGOTerm(const QSqlDatabase &db,
                                   const GeneOntologyTerm &term) {
  for (const auto &id_go : term.getAltID())
    DOGIGO::insertGOTerm(db, id_go, term.getName(), term.getNamespace(),
                         term.getDef(), term.getComment(), term.isObsolete(),
                         term.getID());
  qvec_pair_int result{};
  if (term.hasSlaves())
    for (const auto &id_go : term.getIsA())
      result.append({term.getID(), id_go});
  else
    result.append({term.getID(), 0});

  return result;
}

void DOGIGO::insertGOAnnotation(const QSqlDatabase &db,
                                const QString &id_database, int id_feature,
                                int id_go) {
  QSqlQuery query(db);

  prepare(query,
          "INSERT OR IGNORE INTO GOAnnotations "
          "(id_database, id_feature, id_go) "
          "VALUES (:id_database, :id_feature, :id_go)");

  query.bindValue(":id_database", id_database);
  query.bindValue(":id_feature", id_feature);
  query.bindValue(":id_go", id_go);

  exec(query);
}
