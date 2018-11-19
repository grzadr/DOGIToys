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
  source_identity =
      elements[6] == "-" ? QVariant(QVariant::Int) : elements[6].toInt();
  xref_identity =
      elements[7] == "-" ? QVariant(QVariant::Int) : elements[7].toInt();
  linkage_type = elements[8] == "-" ? QVariant(QVariant::String) : elements[8];
}

void DOGIToys::Mapping::UniprotMapRecord::insert(QSqlDatabase &db) const {
  const auto id_gene = Select::select_id_feature_from_stable_id(db, gene_id);
  if (!id_gene) return;

  auto insert_gene =
      Execute::prepare(db,
                       "INSERT OR IGNORE INTO UniprotMap("
                       "uniprot_xref, id_feature,"
                       "uniprot_db_name, uniprot_info_type,"
                       "uniprot_source_identity, uniprot_xref_identity,"
                       "uniprot_linkage_type) "
                       "VALUES ("
                       ":xref, :id_feature, :db_name, :info_type, "
                       ":source_identity, :xref_identity, :linakge_type)");

  insert_gene.bindValue(":xref", xref);
  insert_gene.bindValue(":id_feature", id_gene);
  insert_gene.bindValue(":db_name", db_name);
  insert_gene.bindValue(":info_type", info_type);
  insert_gene.bindValue(":source_identity", source_identity);
  insert_gene.bindValue(":xref_identity", xref_identity);
  insert_gene.bindValue(":linkage_type", linkage_type);

  Execute::exec(insert_gene);
}
