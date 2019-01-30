#!/bin/bash

set -eux

#rm -rf build
mkdir -p build
cd build
cmake ..
make -j 8

DB_DIR="/Dropbox/DOGI/Databases"
DATA_DIR="/Dropbox/DOGI/Sources"
TAXON="mus_musculus"
ASSEMBLY="Mus_musculus.GRCm38"
ENSEMBL=95

time ./CreateDOGI "${DB_DIR}/${TAXON}.${ENSEMBL}.db" \
  -a "${DATA_DIR}/Ensembl/${ENSEMBL}/GeneAnnotations/${TAXON}/${ASSEMBLY}.${ENSEMBL}.gff3" \
  -f "${DATA_DIR}/Ensembl/${ENSEMBL}/Genomes/${TAXON}/${TAXON}.dna_sm.chroms.fa" \
  -s "${DATA_DIR}/Ensembl/${ENSEMBL}/Variants/${TAXON}/${TAXON}_structural_variations.gvf" \
  -m "${DATA_DIR}/MGI/MGI_Gene_Model_Coord.rpt" \
  -t "${DATA_DIR}/GeneOntology/go.obo" \
  -o "${DATA_DIR}/MGI/gene_association.mgi" \
  
  #   -m "${DATA_DIR}/Ensembl/${ENSEMBL}/Mapping/${TAXON}/${ASSEMBLY}.${ENSEMBL}.uniprot.tsv" \
