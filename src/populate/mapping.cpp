#include <dogitoys/populate/mapping.hpp>

#include <iostream>

DOGIToys::Mapping::UniprotMapRecord::UniprotMapRecord(const QString &line) {
  const auto elements = line.split("\t");

  if (elements.size() != 9)
    throw_runerror("Wrong number of elements in UniprotMap record\n" + line);

  gene_id = elements[0];
  transcript_id = elements[1];
  protein_id = elements[2];
  xref = elements[3];
  db_name = elements[4];
  info_type = elements[5];
  source_identity = elements[6];
  xref_identity = elements[7];
  linkage_type = elements[8];
}

void DOGIToys::Mapping::UniprotMapRecord::insert(QSqlDatabase &db) const {
  const auto id_gene = Select::select_id(db, "GenomicFeatures", "id_feature",
                                         "feature_stable_id", gene_id);
  if (!id_gene) {
    qInfo() << gene_id << id_gene;
  }
}
