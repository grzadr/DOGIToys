#pragma once

#include <QString>
#include <QVector>

#include <agizmo/args.hpp>
#include <dogitoys/tools.hpp>

#include <optional>

using opt_qstr = std::optional<QString>;

struct GeneOntologyFiles {
  QString terms;
  QString annotation;
};

namespace DOGIToys {

class Parameters {
  QString path;

  bool create = false;
  bool create_structural = false;
  bool create_sequences = false;

  opt_qstr taxon;
  opt_qstr genomic_features{};
  opt_qstr ontology_terms{};
  opt_qstr ontology_annotation{};
  opt_qstr uniprot_mapping{};
  opt_qstr mgi_mapping{};
  opt_qstr mapping{};
  QStringList structural_variants{};
  QStringList sequences{};

public:
  Parameters() = default;

  void parse(const AGizmo::Args::Arguments &args);
  void parse(const QString json_file);

  QString str() const;

  QString getPath() const { return path; }
  bool createDOGI() const { return create; }
  bool createStructural() const { return create_structural; }
  bool createSequences() const { return create_sequences; }

  bool hasTaxon() const { return taxon.has_value(); }
  QString getTaxon() const { return *taxon; }

  bool hasFeatures() const { return genomic_features.has_value(); }
  auto getFeatures() const { return *genomic_features; }

  bool hasStructural() const { return !structural_variants.empty(); }
  auto getStructural() const { return structural_variants; }

  bool hasOntologyTerms() const { return ontology_terms.has_value(); }
  auto getOntologyTerms() const { return *ontology_terms; }

  bool hasOntologyAnnotation() const { return ontology_annotation.has_value(); }
  auto getOntologyAnnotation() const { return *ontology_annotation; }

  bool hasUniprotMapping() const { return uniprot_mapping.has_value(); }
  auto getUniprotMapping() const { return *uniprot_mapping; }

  bool hasMGIMapping() const { return mgi_mapping.has_value(); }
  auto getMGIMapping() const { return *mgi_mapping; }

  bool hasMapping() const { return mapping.has_value(); }
  auto getMapping() const { return *mapping; }

  bool hasSequences() const { return sequences.size(); }
  auto getSequences() const { return sequences; }
};

} // namespace DOGIToys
