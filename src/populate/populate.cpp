#include <dogitoys/populate/populate.hpp>

#include <iomanip>

using std::cerr;

void DOGIToys::Populate::Populator::initMain() {
  Initiate::init_main(*db);
  initTaxon();
  initGenomicFeatures();
}

void DOGIToys::Populate::Populator::initTaxon() { Initiate::init_taxon(*db); }

void DOGIToys::Populate::Populator::initGenomicFeatures() {
  Initiate::init_genomic_features(*db);
}

void DOGIToys::Populate::Populator::initSequences() {
  Initiate::init_genomic_sequences(*db);
}

void DOGIToys::Populate::Populator::initUniprotMap() {
  Initiate::init_map_uniprot(*db);
}

void DOGIToys::Populate::Populator::initMGIMap() {
  Initiate::init_map_mgi(*db);
}

void DOGIToys::Populate::Populator::initGeneOntology() {
  Initiate::init_gene_ontology(*db);
}

void DOGIToys::Populate::Populator::initStructuralVariants() {
  Initiate::init_structural_variants(*db);
}

void DOGIToys::Populate::Populator::populateGeneOntologyHierarchy(
    const QVector<QPair<int, int>> hierarchy) {
  transaction();

  for (const auto &[go_id, go_parent] : hierarchy)
    GeneOntology::insert_go_hierarchy(*db, go_id, go_parent);

  commit();
}

void DOGIToys::Populate::Populator::populateGenomicFeatures(QString gff3_file,
                                                            bool initiate) {
  qInfo() << "Populating Genomic Features";

  if (initiate)
    initGenomicFeatures();

  qInfo() << "File:" << gff3_file;

  HKL::GFF::GFFReader reader(gff3_file.toStdString());

  Transaction::transaction(*db);

  int id_record = Select::select_max_id(*db, "GenomicFeatures", "id_feature");
  int id_child =
      Select::select_max_id(*db, "GenomicFeaturesChildren", "id_feature_child");

  while (auto record = reader()) {
    if ((*record).index() == 0) {
      const auto &comment = std::get<0>(*record);
      if (const auto &seqid = comment.getRegion())
        insert_SeqID(*db, QString::fromStdString((*seqid).getChrom()),
                     (*seqid).getFirst(), (*seqid).getLast());
    } else {
      GenomicFeature feature(std::move(std::get<1>(*record)));
      if (feature.hasParent())
        feature.insert(*db, ++id_record);
      else
        feature.insert(*db, ++id_child);

      if (id_record % 50000 == 0)
        qInfo() << id_record;
    }
  }

  Transaction::commit(*db);
}

void DOGIToys::Populate::Populator::populateSequences(QStringList fasta_files,
                                                      bool initiate) {
  if (initiate || !db->tables().contains("Sequences"))
    initSequences();

  for (const auto &fasta_file : fasta_files)
    populateSequences(fasta_file, false);
}

void DOGIToys::Populate::Populator::populateSequences(QString fasta_file,
                                                      bool initiate) {
  qInfo() << "Populating GenomicSequences";
  qInfo() << "Reading FASTA" << fasta_file;

  if (initiate || !db->tables().contains("Sequences"))
    initSequences();

  HKL::FASTAReader reader(fasta_file.toStdString());

  Transaction::transaction(*db);

  while (const auto seq = reader.readSeq()) {
    insertGenomicSequence(*seq);
  }

  Transaction::commit(*db);
}

void DOGIToys::Populate::Populator::insertGenomicSequence(
    const HKL::RegionSeq &seq) {
  auto insert = Execute::prepare(*db, "INSERT INTO Sequences("
                                      "id_sequence,"
                                      "sequence_seq, sequence_length) "
                                      "VALUES (:id, :seq, :length)");
  insert.bindValue(":id", QString::fromStdString(seq.getName()));
  insert.bindValue(":seq", QString::fromStdString(seq.getSeq()));
  insert.bindValue(":length", static_cast<int>(seq.size()));

  Execute::exec(insert);
}

void DOGIToys::Populate::Populator::populateMap(const QString map_file,
                                                bool overwrite) {
  if (map_file.endsWith(".rpt"))
    populateMGIMap(map_file, overwrite);
  else
    populateUniprotMap(map_file, overwrite);
}

void DOGIToys::Populate::Populator::populateUniprotMap(const QString map_file,
                                                       bool overwrite) {
  qInfo() << "Populating Uniprot Mappings";

  if (overwrite || !db->tables().contains("UniprotMap"))
    initUniprotMap();

  transaction();

  AGizmo::Files::FileReader reader(map_file.toStdString());

  reader();

  while (const auto line = reader()) {
    Mapping::UniprotMapRecord record(QString::fromStdString(*line));
    record.insert(*db);
  }

  qInfo() << "Commiting";

  commit();
}

void DOGIToys::Populate::Populator::populateMGIMap(const QString map_file,
                                                   bool overwrite) {
  if (overwrite || !db->tables().contains("MGIMap"))
    initMGIMap();

  QFileReader reader(map_file);
  reader(1);

  auto insert = Execute::prepare(*db, "INSERT INTO "
                                      "MGIMap (id_mgi, id_feature) "
                                      "VALUES (:id_mgi, :id_feature)");

  transaction();

  while (auto line = reader()) {
    auto data = (*line).split("\t");
    const auto id_mgi = extractID(data[1], ':');
    if (const auto id_feature =
            Select::select_id_feature_from_stable_id(*db, data[10]);
        id_feature) {
      insert.bindValue(":id_mgi", id_mgi);
      insert.bindValue(":id_feature", id_feature);
      Execute::exec(insert);
    }
  }

  commit();
}

void DOGIToys::Populate::Populator::populateGeneOntologyTerms(
    const QString &obo_file, bool overwrite) {
  if (overwrite || !db->tables().contains("GeneOntologyTerms"))
    initGeneOntology();

  qInfo() << "Populating GeneOntology Terms";
  transaction();

  GeneOntology::OBOParser obo_parser(obo_file);

  QVector<QPair<int, int>> hierarchy{};

  while (auto term = obo_parser.getTerm()) {
    (*term).insert(*db);
    if ((*term).hasParents())
      std::transform((*term).getIsA().begin(), (*term).getIsA().end(),
                     std::back_inserter(hierarchy), [&term](int is_a) {
                       return QPair<int, int>{(*term).getID(), is_a};
                     });
    else
      hierarchy.append({(*term).getID(), 0});
  }

  commit();

  populateGeneOntologyHierarchy(hierarchy);
}

void DOGIToys::Populate::Populator::populateGeneOntologyAnnotation(
    const QString file_name, bool overwrite) {
  populateGeneOntologyAnnotation(file_name, overwrite,
                                 file_name.endsWith(".mgi"));
}

void DOGIToys::Populate::Populator::populateGeneOntologyAnnotation(
    const QString mgi_file, bool overwrite, bool from_mgi) {
  if (overwrite)
    Execute::exec(*db, "DELETE FROM GeneOntologyAnnotation");

  QFileReader reader(mgi_file);

  transaction();

  while (auto line = reader("!")) {
    auto record = GeneOntology::GAFRecord(*line, from_mgi);
    record.insert(*db);
  }

  commit();
}

void DOGIToys::Populate::Populator::populateStructuralVariants(
    const QStringList &gvf_files, bool overwrite) {
  if (overwrite || !db->tables().contains("StructuralVariants"))
    initStructuralVariants();

  qInfo() << "Populating Structural Variants";

  for (const auto &gvf_file : gvf_files) {
    populateStructuralVariants(gvf_file, false);
  }
}

void DOGIToys::Populate::Populator::populateStructuralVariants(
    const QString &gvf_file, bool overwrite) {
  if (overwrite || !db->tables().contains("StructuralVariants"))
    initStructuralVariants();

  qInfo() << "Populating Structural Variants";

  transaction();

  int inserted = 0;
  int id_record = Select::select_max_id(*db, "StructuralVariants", "id_struct");
  int id_child = Select::select_max_id(*db, "StructuralVariantsChildren",
                                       "id_struct_child");

  HKL::GFF::GFFReader reader(gvf_file.toStdString());
  while (const auto record = reader("#")) {
    StructuralVariant temp(std::get<GFF::GFFRecord>(*record));
    if (temp.hasParent())
      temp.insertChild(*db, ++id_child);
    else
      temp.insert(*db, ++id_record);
    if (++inserted % 100000 == 0)
      qInfo() << inserted;
  }

  commit();
}

void DOGIToys::Populate::Populator::populate(
    const DOGIToys::Parameters &params) {

  if (params.hasFeatures())
    populateGenomicFeatures(params.getFeatures(), params.createDOGI());

  if (params.hasMapping())
    populateMap(params.getMapping(), params.createDOGI());
  else if (params.hasUniprotMapping())
    populateUniprotMap(params.getUniprotMapping(), params.createDOGI());
  else if (params.hasMGIMapping())
    populateGenomicFeatures(params.getFeatures(), params.createDOGI());

  if (params.hasOntologyTerms())
    populateGeneOntologyTerms(params.getOntologyTerms(), params.createDOGI());
  if (params.hasOntologyAnnotation())
    populateGeneOntologyAnnotation(params.getOntologyAnnotation(),
                                   params.createDOGI());

  if (params.hasStructural()) {
    populateStructuralVariants(params.getStructural(),
                               params.createStructural());
  }

  if (params.hasSequences()) {
    populateSequences(params.getSequences(), params.createSequences());
  }
}
