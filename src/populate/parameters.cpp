#include <dogitoys/populate/parameters.hpp>

void DOGIToys::Parameters::parse(const AGizmo::Args::Arguments& args) {
  for (const auto& flag : args) {
    if (const auto flag_name = flag.getName();
        flag_name == "name" || flag_name == "n") {
      if (flag.isEmpty())
        throw_runerror("Input file not specifed, but --input/-i flag set!");
      name = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "taxon" || flag_name == "t") {
      if (flag.isEmpty())
        throw_runerror("Taxon not specifed, but --taxon/-t flag set!");
      taxon = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "structural" || flag_name == "s") {
      if (flag.isEmpty())
        throw_runerror(
            "Input file not specifed, but --structural/-s flag set!");
      structural_variants = QString::fromStdString(*flag.getValue());
    } else if (flag_name == "structural" || flag_name == "s") {
      if (flag.isEmpty())
        throw_runerror("Input file not specifed, but --map/-m flag set!");
      features_map.append(QString::fromStdString(*flag.getValue()));
    } else {
      throw_runerror("Unknown flag '" + name + "'");
    }
  }
}
