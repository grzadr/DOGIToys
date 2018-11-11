#!/usr/bin/python3

from sys import path, argv

path.append(".")

from pyDOGIToys import DOGI

def main():
    input_dir = argv[1]

    print("Testing pyDOGIToys")
    print("Create basic connector")
    db = DOGI();
    print("Done!\n")
    print("Test - Creating new database")
    db.open("temp.db", True)
    print("Done!\n")
    print("Test - Closing connection")
    db.close(True);
    print("Done!\n")
    print("Test - Opening connection")
    db.open("temp.db")
    print("Done!\n")
    print("Test - setTaxon('human')")
    db.setTaxon("human")
    print(db.getTaxonName())
    print("Test - Done!\n")
    print("Test - setTaxon(9615)")
    db.setTaxon(9615, True)
    print(db.getTaxonName())
    print("Test - Done!\n")

    # db.populateGenomicFeatures(input_dir + "/annotation.gff")
    db.populateGenomicFeatures("/Dropbox/Dropbox/DOGI/Sources/Ensembl/"
                               "94/GeneAnnotations/homo_sapiens/"
                               "Homo_sapiens.GRCh38.94.gff3")
main()
