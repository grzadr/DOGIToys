#include <dogitoys/populate.hpp>

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
  qWarning() << db->tables();
}

void DOGIToys::Populate::Populator::populateGenomicFeatures(QString gff3_file) {
  qInfo() << "Populating Genomic Features";

  qInfo() << "File:" << gff3_file;

  HKL::GFF::GFFReader reader(gff3_file.toStdString());

  db->transaction();

  while (auto record = reader()) {
    if ((*record).index() == 0) {
      const auto& comment = std::get<0>(*record);
      if (const auto& seqid = comment.getRegion())
        insert_SeqID(*db, QString::fromStdString((*seqid).getChrom()),
                     (*seqid).getFirst(), (*seqid).getLast(),
                     (*seqid).getStrand());
    } else {
      GenomicFeature feature(std::move(std::get<1>(*record)));
      feature.insert(*db);
    }
  }

  db->commit();
}

void DOGIToys::Populate::Populator::populateFASTA(QString fasta_file) {}
