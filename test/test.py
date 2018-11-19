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

    test = False
    initiate=False

    if test:
        db_path = "test.db"
        print("Test - Creating new database")
        db.open(db_path, True)
        print("Done!\n")
        print("Test - Closing connection")
        db.close(True);
        print("Done!\n")
        print("Test - Opening connection")
        db.open(db_path)
        print("Done!\n")

        print("Test - setTaxon(9615)")
        db.setTaxon(9615, True)
        print(db.getTaxonName())
        print("Test - Done!\n")

        print("Test - setTaxon('human')")
        db.setTaxon("human", True)
        print(db.getTaxonName())
        print("Test - Done!\n")
    else:
        db.open("/Dropbox/DOGI/temp.db", initiate)
        db.setTaxon("human", True)

    if test:
        annotation = input_dir + "/annotation.gff"
        uniprot_map = None
        gene_ontology_terms = None
    else:
        annotation = ("/Dropbox/DOGI/Sources/Ensembl/94/"
                      "GeneAnnotations/homo_sapiens/"
                      "Homo_sapiens.GRCh38.94.gff3")
        uniprot_map = ("/Dropbox/DOGI/Sources/Ensembl/94/Mapping/homo_sapiens/"
                       "Homo_sapiens.GRCh38.94.uniprot.tsv")
        gene_ontology_terms = "/cloud/Dropbox/Dropbox/DOGI/Sources/GeneOntology/go.obo"

    if test or initiate:
        db.populateGenomicFeatures(annotation, True)

    if uniprot_map is not None:
        db.populateUniprotMap(uniprot_map, True)

    if gene_ontology_terms is not None:
        db.populateGeneOntologyTerms(gene_ontology_terms, True)
    # db.populateGenomicFeatures("/Dropbox/DOGI/Sources/Ensembl/"
    #                            "94/GeneAnnotations/homo_sapiens/"
    #                            "Homo_sapiens.GRCh38.94.gff3")

main()
