#include <dogitoys/connect.hpp>
#include <dogitoys/create.hpp>
#include <dogitoys/populate/parameters.hpp>

int main(int argc, char *argv[]) {
  AGizmo::Args::Arguments args{};
  args.addPositional("path", "Path do database file", true);
  args.addPositional("taxon", "Taxon ID/Name", false);
  args.addSwitch("create", "Force database creation", 'c');
  args.addArgument("features",
                   "File with genomic features. Forces database creation", 'a');
  args.addSwitch("init-structural",
                 "Force initialization of StructuralVariants table");
  args.addMulti("structural", "File with structural variants", 's');
  args.addArgument("ontology-terms", "File with ontology terms", 't');
  args.addArgument("ontology", "File with ontology annotation", 'o');
  args.addArgument("uniprot-mapping", "Mapping for Uniprot", 'u');
  args.addArgument("mgi-mapping", "Mapping for Uniprot", 'i');
  args.addArgument("mapping", "Mapping", 'm');
  args.addMulti("fasta", "FASTA with genomic sequences", 'f');

  args.parse(argc, argv);

  DOGIToys::Parameters params{};

  params.parse(args);

  DOGIToys::DOGI dogi{params};
}
