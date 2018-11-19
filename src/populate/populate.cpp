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
  Initiate::init_uniprot_map(*db);
}

void DOGIToys::Populate::Populator::initGeneOntology() {
  Initiate::init_gene_ontology(*db);
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

  Transaction::transaction(*db);

  AGizmo::Files::FileReader reader(map_file.toStdString());

  reader();

  while (const auto line = reader()) {
    Mapping::UniprotMapRecord record(QString::fromStdString(*line));
    record.insert(*db);
  }

  qInfo() << "Commiting";

  Transaction::commit(*db);
}

void DOGIToys::Populate::Populator::populateGeneOntologyTerms(
    const QString obo_file, bool overwrite) {
  if (overwrite || !db->tables().contains("GeneOntologyTerms"))
    initGeneOntology();

  qInfo() << "Populating Uniprot Mappings";
  Transaction::transaction(*db);

  Transaction::commit(*db);
}
