#include <dogitoys/populate/parameters.hpp>

using namespace DOGIToys;

void Parameters::parse(const AGizmo::Args::Arguments &args) {

  this->path = QString::fromStdString(*args.getValue("path"));

  if (auto taxon = args.getValue("taxon"))
    this->taxon = QString::fromStdString(*taxon);

  this->create = args.isSet("create");

  if (auto genomic_features = args.getValue("features")) {
    this->genomic_features = QString::fromStdString(*genomic_features);
    this->create = true;
  }

  if (auto structural_variants = args.getValue("structural"))
    this->structural_variants = QString::fromStdString(*structural_variants);

  if (auto uniprot_mapping = args.getValue("uniprot-mapping"))
    this->uniprot_mapping = QString::fromStdString(*uniprot_mapping);

  if (auto mgi_mapping = args.getValue("mgi-mapping"))
    this->mgi_mapping = QString::fromStdString(*mgi_mapping);

  if (auto mapping = args.getValue("mapping"))
    this->mapping = QString::fromStdString(*mapping);

  if (auto ontology_terms = args.getValue("ontology-terms"))
    this->ontology_terms = QString::fromStdString(*ontology_terms);

  if (auto ontology_annotation = args.getValue("ontology"))
    this->ontology_annotation = QString::fromStdString(*ontology_annotation);

  for (auto sequence : args.getIterable("fasta")) {
    sequences.append(QString::fromStdString(sequence));
  }
}

QString DOGIToys::Parameters::str() const {
  QString result = "DB path:" + path + "\n";
  result += "Taxon: " + (taxon.has_value() ? *taxon : "None");
  result += "Features: " +
            (genomic_features.has_value() ? *genomic_features : "None");
  result += "Structural: " +
            (structural_variants.has_value() ? *structural_variants : "None");
  result += "Ontology Terms: " +
            (ontology_terms.has_value() ? *ontology_terms : "None");
  result += "Ontology Annotation: " +
            (ontology_annotation.has_value() ? *ontology_annotation : "None");
  result += "Uniprot Mapping: " +
            (uniprot_mapping.has_value() ? *uniprot_mapping : "None");
  result += "MGI Mapping: " + (mgi_mapping.has_value() ? *mgi_mapping : "None");
  result += "Sequences: " + (sequences.size() ? sequences.join(";") : "None");

  return result;
}
