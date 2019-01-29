#!/bin/bash

set -eux

#rm -rf build
mkdir -p build
cd build
cmake ..
make -j 8

DB_DIR="/Dropbox/DOGI/Databases"
DATA_DIR="/Dropbox/DOGI/Sources"

time ./CreateDOGI "${DB_DIR}/human.db" \
  -f "${DATA_DIR}/Ensembl/95/GeneAnnotations/homo_sapiens/Homo_sapiens.GRCh38.95.gff3"
