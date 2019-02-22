#!/bin/bash

set -eux

#rm -rf build
mkdir -p build
cd build
cmake ..
make -j 8

DB_DIR="/Dropbox/MyPhD/DOGI/Databases/"
DATA_DIR="/Dropbox/MyPhD/DOGI/Sources"

ENSEMBL=95

TAXON_LIST=("bos_taurus" "canis_familiaris" "homo_sapiens" "sus_scrofa")
ASSEMBLIES_LIST=("Bos_taurus.ARS-UCD1.2" "Canis_familiaris.CanFam3.1" 
                 "Homo_sapiens.GRCh38" "Sus_scrofa.Sscrofa11.1")

for i in ${!TAXON_LIST[@]}; do
  TAXON="${TAXON_LIST[$i]}"
  ASSEMBLY="${ASSEMBLIES_LIST[$i]}"

#   echo "${i} ${TAXON} ${ASSEMBLY}"
  
  time ./CreateDOGI "${DB_DIR}/${TAXON}.${ENSEMBL}.db" \
    -a "${DATA_DIR}/Ensembl/${ENSEMBL}/GeneAnnotations/${TAXON}/${ASSEMBLY}.${ENSEMBL}.gff3" \
    -f "${DATA_DIR}/Ensembl/${ENSEMBL}/Genomes/${TAXON}/${TAXON}.dna_sm.chroms.fa" \
    -s "${DATA_DIR}/Ensembl/${ENSEMBL}/Variants/${TAXON}/${TAXON}_structural_variations.gvf" \
    -t "${DATA_DIR}/GeneOntology/go.obo" \
    -o "${DATA_DIR}/GeneOntology/${TAXON}.gaf" \
    -m "${DATA_DIR}/Ensembl/${ENSEMBL}/Mapping/${TAXON}/${ASSEMBLY}.${ENSEMBL}.uniprot.tsv"
done

TAXON="mus_musculus"
ASSEMBLY="Mus_musculus.GRCm38"
  
time ./CreateDOGI "${DB_DIR}/${TAXON}.${ENSEMBL}.db" \
  -a "${DATA_DIR}/Ensembl/${ENSEMBL}/GeneAnnotations/${TAXON}/${ASSEMBLY}.${ENSEMBL}.gff3" \
  -f "${DATA_DIR}/Ensembl/${ENSEMBL}/Genomes/${TAXON}/${TAXON}.dna_sm.chroms.fa" \
  -s "${DATA_DIR}/Ensembl/${ENSEMBL}/Variants/${TAXON}/${TAXON}_structural_variations.gvf" \
  -m "${DATA_DIR}/MGI/MGI_Gene_Model_Coord.rpt" \
  -t "${DATA_DIR}/GeneOntology/go.obo" \
  -o "${DATA_DIR}/MGI/gene_association.mgi"
