#include <dogitoys/create.hpp>
#include <dogitoys/populate/parameters.hpp>

using ArgsValueType = AGizmo::Args::ValueType;

int main(int argc, char *argv[]) {
  AGizmo::Args::Arguments args{};
  args.addArgument(1, "path", "Path do database file", ArgsValueType::String);
  args.addArgument(2, "taxon", "Taxon ID/Name", ArgsValueType::String);
  args.addArgument(3, "features", "File with genomic features",
                   ArgsValueType::String);
  args.addArgument("structural", "File with structural variants",
                   ArgsValueType::String, 's');
  args.addArgument("ontology-terms", "File with ontology annotation",
                   ArgsValueType::String, 'o');
  args.addArgument("ontology-annotation", "File with ontology terms",
                   ArgsValueType::String, 'a');
  args.addArgument("uniprot-mapping", "Mapping for Uniprot mapping",
                   ArgsValueType::String, 'u');
  args.addArgument("fasta", "FASTA with genomic sequences",
                   ArgsValueType::String, 'f');

  args.parse(argc, argv);

  DOGIToys::Parameters params{};

  params.parse(args);
}
