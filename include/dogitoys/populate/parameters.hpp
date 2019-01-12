#pragma once

#include <QString>
#include <QVector>

#include <agizmo/args.hpp>
#include <dogitoys/tools.hpp>

#include <optional>

struct GeneOntologyFiles {
  QString terms;
  QString annotation;
};

namespace DOGIToys {

class Parameters {
  QString name;

  std::optional<QString> taxon;
  std::optional<QString> genomic_features{};
  std::optional<QString> structural_variants{};
  std::optional<QString> ontology_terms{};
  std::optional<QString> ontology_annotation{};
  QVector<QString> features_map{};

  void parse(const AGizmo::Args::Arguments &args);
  void parse(int argc, char *argv[]) {
    //      const auto args = AGizmo::Args::Arguments(argc, argv);
    //    parse(AGizmo::Args::Arguments(argc, argv));
  }
  void parse(const QString json_file);

  Parameters() = default;
};

} // namespace DOGIToys
