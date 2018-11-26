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

void DOGIToys::Populate::Populator::initGenomicSequences() {
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

void DOGIToys::Populate::Populator::populateGeneOntologyHierarchy(
    const QVector<QPair<int, int>> hierarchy) {
  transaction();

  for (const auto& [go_id, go_parent] : hierarchy)
    GeneOntology::insert_go_hierarchy(*db, go_id, go_parent);

  commit();
}

void DOGIToys::Populate::Populator::populateGenomicFeatures(QString gff3_file,
                                                            bool initiate) {
  qInfo() << "Populating Genomic Features";

  if (initiate) initGenomicFeatures();

  qInfo() << "File:" << gff3_file;

  HKL::GFF::GFFReader reader(gff3_file.toStdString());

  Transaction::transaction(*db);

  while (auto record = reader()) {
    if ((*record).index() == 0) {
      const auto& comment = std::get<0>(*record);
      if (const auto& seqid = comment.getRegion())
        insert_SeqID(*db, QString::fromStdString((*seqid).getChrom()),
                     (*seqid).getFirst(), (*seqid).getLast());
    } else {
      GenomicFeature feature(*db, std::move(std::get<1>(*record)));
      if (const auto id_feature = feature.insert(); id_feature % 50000 == 0)
        qInfo() << id_feature;
    }
  }

  Transaction::commit(*db);
}

void DOGIToys::Populate::Populator::populateGenomicSequences(QString fasta_file,
                                                             QString masking,
                                                             bool initiate) {
  qInfo() << "Populating GenomicSequences";
  qInfo() << "Reading FASTA" << fasta_file;

  masking = masking.toLower();
  if (masking != "hard" && masking != "soft" && masking != "none")
    throw_runerror("Unsupported masking: " + masking);

  if (initiate) initGenomicSequences();

  HKL::FASTAReader reader(fasta_file.toStdString());

  Transaction::transaction(*db);

  while (const auto seq = reader.readSeq()) {
    insertGenomicSequence(*seq, masking);
  }

  Transaction::commit(*db);
}

void DOGIToys::Populate::Populator::insertGenomicSequence(
    const HKL::RegionSeq& seq, const QString masking) {
  auto insert = Execute::prepare(*db,
                                 "INSERT INTO GenomicSequences("
                                 "id_sequence, sequence_masking,"
                                 "sequence_seq, sequence_length) "
                                 "VALUES (:id, :masking, :seq, :length)");
  insert.bindValue(":id", QString::fromStdString(seq.getName()));
  insert.bindValue(":masking", masking);
  insert.bindValue(":seq", QString::fromStdString(seq.getSeq()));
  insert.bindValue(":length", static_cast<int>(seq.size()));

  Execute::exec(insert);
}

void DOGIToys::Populate::Populator::populateUniprotMap(const QString map_file,
                                                       bool overwrite) {
  qInfo() << "Populating Uniprot Mappings";

  if (overwrite || !db->tables().contains("UniprotMap")) initUniprotMap();

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
  if (overwrite || !db->tables().contains("MGIMap")) initMGIMap();

  QFileReader reader(map_file);
  reader(1);

  auto insert = Execute::prepare(*db,
                                 "INSERT INTO "
                                 "MGIMap (id_mgi, id_feature) "
                                 "VALUES (:id_mgi, :id_feature)");

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
}

void DOGIToys::Populate::Populator::populateGeneOntologyTerms(
    const QString& obo_file, bool overwrite) {
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
  if (overwrite) Execute::exec(*db, "DELETE FROM GeneOntologyAnnotation");

  QFileReader reader(mgi_file);

  transaction();

  while (auto line = reader("!")) {
    auto record = GeneOntology::GAFRecord(*line, from_mgi);
    record.insert(*db);
  }

  commit();
}
