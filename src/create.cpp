#include <dogitoys/connect.hpp>
#include <dogitoys/create.hpp>
#include <dogitoys/populate/parameters.hpp>

using avt = AGizmo::Args::ValueType;

int main(int argc, char *argv[]) {
  AGizmo::Args::Arguments args{};
  args.addArgument("path", "Path do database file", true);
  args.addArgument("taxon", "Taxon ID/Name", false);
  args.addArgument("create", "Force database creation", avt::Bool, 'c');
  args.addArgument("features",
                   "File with genomic features. Forces database creation",
                   avt::Single, 'f');
  args.addArgument("structural", "File with structural variants", avt::Single,
                   's');
  args.addArgument("ontology-terms", "File with ontology terms", avt::Single,
                   't');
  args.addArgument("ontology", "File with ontology annotation", avt::Single,
                   'o');
  args.addArgument("uniprot-mapping", "Mapping for Uniprot", avt::Single, 'u');
  args.addArgument("mgi-mapping", "Mapping for Uniprot", avt::Single, 'i');
  args.addArgument("mapping", "Mapping", avt::Single, 'm');
  args.addArgument("fasta", "FASTA with genomic sequences", avt::Single, 'f');

  args.parse(argc, argv);

  DOGIToys::Parameters params{};

  params.parse(args);

  DOGIToys::DOGI dogi{params};
}
