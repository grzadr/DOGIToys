#include "dogitoys-snack.h"

using namespace DOGIToys;

opt_qstr DOGISnack::loadFile(QString field, bool obligatory) {
  if (!conf.contains(field)) {
    if (obligatory)
      throw_error("Obligatory field not provied " + field);
    else
      return {};
  }

  const auto path = conf[field].toString();

  if (!QFileInfo::exists(path))
    throw_error(field + " -> File is not readable: '" + path + "'");

  provided.append(std::move(field));

  return std::move(path);
}

optional<QVector<GFFFile>> DOGISnack::loadAnnotation(QString field,
                                                     bool obligatory) {
  if (!conf.contains(field)) {
    if (obligatory)
      throw_error("Obligatory field not provied " + field);
    else
      return {};
  }

  QVector<GFFFile> result{};

  for (const auto ele : conf[field].toArray()) {
    const auto &temp = ele.toObject();

    if (QString path = temp["path"].toString(); !QFileInfo::exists(path))
      throw_error(field + " -> File doesnt exists '" + path + "'");
    else
      result.append({temp["database"].toString(), std::move(path)});
  }

  provided.append(std::move(field));

  return std::move(result);
}

optional<QVector<Mapper>> DOGISnack::loadMapper(QString field,
                                                bool obligatory) {
  if (!conf.contains(field)) {
    if (obligatory)
      throw_error("Obligatory field not provied " + field);
    else
      return {};
  }

  QVector<Mapper> result{};

  for (const auto ele : conf[field].toArray()) {
    const auto &temp = ele.toObject();

    if (QString path = temp["path"].toString(); !QFileInfo::exists(path))
      throw_error(field + " -> File doesnt exists '" + path + "'");
    else
      result.append(
          {temp["from"].toString(), temp["to"].toString(), std::move(path)});
  }

  provided.append(std::move(field));

  return std::move(result);
}

optional<QVector<Genome>> DOGISnack::loadGenome(QString field,
                                                bool obligatory) {
  if (!conf.contains(field)) {
    if (obligatory)
      throw_error("Obligatory field not provied " + field);
    else
      return {};
  }

  QVector<Genome> result{};

  for (const auto ele : conf[field].toArray()) {
    const auto &temp = ele.toObject();

    if (QString path = temp["path"].toString(); !QFileInfo::exists(path))
      throw_error(field + " -> File doesnt exists '" + path + "'");
    else
      result.append({temp["database"].toString(), temp["masking"].toString(),
                     std::move(path)});
  }

  provided.append(std::move(field));

  return std::move(result);
}

// void DOGISnack::load_taxonomy(const QJsonObject &conf) {
//  QString ele{"taxonomy"};
//  if (!conf.contains(ele))
//    throw runtime_error("Obligatory taxonomy file not provied.");

//  taxonomy.setFileName(conf[ele].toString());

//  if (!taxonomy.exists())
//    throw_error("Taxonomy file is not readable: " + taxonomy.fileName());
//  provided.append(std::move(ele));
//}

void DOGISnack::load_annotations(const QJsonObject &conf) {
  QString ele{"annotations"};
  if (!conf.contains(ele))
    return;

  for (const auto ele : conf[ele].toArray()) {
    const auto &annotation = ele.toObject();

    if (QString gff3_file = annotation["path"].toString();
        !QFileInfo::exists(gff3_file))
      throw_error("GFF3 File doesnt exists " + gff3_file);
    else
      annotations.append({annotation["database"].toString(), gff3_file});
  }

  provided.append(std::move(ele));
}

void DOGISnack::loadAnnotations() {
  if (auto temp = loadAnnotation("annotations", true))
    annotations = std::move(*temp);
}

void DOGISnack::loadTaxonomy() {
  if (auto temp = loadFile("taxonomy", false))
    go_terms = std::move(*temp);
}

void DOGISnack::loadNCBI2Ensembl() {
  if (auto temp = loadFile("ncbi2ensembl", false))
    ncbi2ensembl.setFileName(std::move(*temp));
}

void DOGISnack::loadHPAExpression() {
  if (auto temp = loadFile("hpa_expression", false))
    hpa_expression = std::move(*temp);
}

void DOGISnack::loadGOTerms() {
  if (auto temp = loadFile("go_terms", false))
    go_terms = std::move(*temp);
}

void DOGISnack::loadGOAnnotations() {
  if (auto temp = loadFile("go_annotations", false))
    go_annotations = std::move(*temp);
}

void DOGISnack::loadMappers() {
  if (auto temp = loadMapper("mapper", false))
    mappers = std::move(*temp);
}

void DOGISnack::loadGenomes() {
  if (auto temp = loadGenome("genome", false))
    genomes = std::move(*temp);
}

void DOGISnack::loadGVF() {
  if (auto temp = loadAnnotation("gvf", false))
    gvf = std::move(*temp);
}

void DOGISnack::loadStructural() {
  if (auto temp = loadAnnotation("structural_variants", false))
    structural = std::move(*temp);
}

void DOGISnack::loadRegulatory() {
  if (auto temp = loadAnnotation("regulatory", false))
    regulatory = std::move(*temp);
}

DOGISnack::DOGISnack(const QString &path) {
  if (!QFileInfo::exists(path))
    throw_error("Config file does not exists: " + path);

  QFile configation(path);

  if (!configation.open(QIODevice::ReadOnly))
    throw runtime_error("Couldn't open config file: " + path.toStdString());

  QJsonParseError error;
  conf = QJsonDocument::fromJson(configation.readAll(), &error).object();

  if (error.error)
    throw_error("Error while reading JSON file\n" + error.errorString());

  //  load_taxonomy(conf);
  organism = conf["organism"].toString();
  //  load_annotations(conf);

  loadAnnotations();
  loadTaxonomy();
  loadHPAExpression();
  loadNCBI2Ensembl();
  loadGOTerms();
  loadGOAnnotations();
  loadMappers();
  loadGenomes();
  loadGVF();
  loadRegulatory();
}
