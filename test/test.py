#!/usr/bin/python3

from sys import path, argv

path.append(".")

from pyDOGIToys import DOGI

def main():
    input_dir = argv[1]

    test = True
    db_path = "temp.db" if test else "/Git/GitHub/DOGIToys/test/input/temp.db"

    print("Testing pyDOGIToys")
    print("Create basic connector")
    db = DOGI();
    print("Done!\n")
    print("Test - Creating new database")
    db.open(db_path, True)
    print("Done!\n")
    print("Test - Closing connection")
    db.close(True);
    print("Done!\n")
    print("Test - Opening connection")
    db.open(db_path)
    print("Done!\n")
    print("Test - setTaxon('human')")
    db.setTaxon("human")
    print(db.getTaxonName())
    print("Test - Done!\n")
    print("Test - setTaxon(9615)")
    db.setTaxon(9615, True)
    print(db.getTaxonName())
    print("Test - Done!\n")

    if test:
        annotation = input_dir + "/annotation.gff"
    else:
        annotation = "/Dropbox/DOGI/Sources/Ensembl/94/"
        "GeneAnnotations/homo_sapiens/"
        "Homo_sapiens.GRCh38.94.gff3"

    db.populateGenomicFeatures(input_dir + "/annotation.gff")
    #db.populateGenomicFeatures("/Dropbox/DOGI/Sources/Ensembl/"
                               #"94/GeneAnnotations/homo_sapiens/"
                               #"Homo_sapiens.GRCh38.94.gff3")

    db.populateGenomicSequences("/Dropbox/DOGI/Sources/Ensembl/94/Genomes/homo_sapiens/test.fa", "none", True)
main()
