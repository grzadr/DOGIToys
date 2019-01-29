#include <dogitoys/populate/parameters.hpp>

using namespace DOGIToys;

void Parameters::parse(const AGizmo::Args::Arguments &args) {
  for (const auto &flag : args) {
    if (flag.isEmpty())
      continue;

    if (const auto flag_name = flag.getName(); flag_name == "path")
      path = QString::fromStdString(*flag.getValue());
    else if (flag_name == "taxon")
      taxon = QString::fromStdString(*flag.getValue());
    else if (flag_name == "create")
      create = true;
    else if (flag_name == "features") {
      genomic_features = QString::fromStdString(*flag.getValue());
      create = true;
    } else if (flag_name == "structural") {
      structural_variants = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "uniprot-mapping") {
      uniprot_mapping = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "mgi-mapping") {
      mgi_mapping = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "mapping") {
      mapping = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "ontology-terms") {
      ontology_terms = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "ontology") {
      ontology_annotation = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "fasta") {
      sequences.append(QString::fromStdString(*flag.getValue()));
    }
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
