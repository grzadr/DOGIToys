#include <dogitoys/populate.hpp>

void DOGIToys::Populate::Populator::initMain() {
  Initiate::init_main(*db);
  Initiate::init_taxon(*db);
}

void DOGIToys::Populate::Populator::init_sequences() {
  qWarning() << db->tables();
}

void DOGIToys::Populate::Populator::populateFASTA(QString fasta_file) {}
