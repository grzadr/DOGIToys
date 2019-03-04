#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "dogitoys/connect.hpp"

namespace py = pybind11;
using namespace pybind11::literals;
using namespace DOGIToys;

using std::ifstream;
using std::string;

PYBIND11_MODULE(pyDOGIToys, m) {
  py::class_<DOGI>(m, "DOGI")
      // Constructors
      .def(py::init<>())
      .def(py::init<const string &, const string &>(), "path"_a,
           "config"_a = "")
      .def("open", py::overload_cast<const string &, bool>(&DOGI::open),
           "path"_a, "create"_a = false)
      .def("close", &DOGI::close, "integrity_check"_a = true,
           "optimize"_a = true)
      .def("destroy", &DOGI::destroy, "confirm"_a = false)

      .def("clear_taxon", &DOGI::clear_taxon)

      .def("getIdTaxon", &DOGI::getIdTaxon)
      .def("getTaxonName", &DOGI::getTaxonName)

      .def("setTaxon", py::overload_cast<>(&DOGI::setTaxon))
      .def("setTaxon", py::overload_cast<int, bool>(&DOGI::setTaxon),
           "id_taxon"_a, "overwrite"_a = false)
      .def("setTaxon", py::overload_cast<string, bool>(&DOGI::setTaxon),
           "name"_a, "overwrite"_a = false)

      .def("populateGenomicFeatures",
           py::overload_cast<string, bool>(&DOGI::populateGenomicFeatures),
           "gff3_file"_a, "initiate"_a = true)

      .def("populateGenomicSequences",
           py::overload_cast<string, bool>(&DOGI::populateSequences),
           "fasta_file"_a, "overwrite"_a = false)

      .def("populateUniprotMap",
           py::overload_cast<string, bool>(&DOGI::populateUniprotMap),
           "map_file"_a, "overwrite"_a = true)

      .def("populateGeneOntologyTerms",
           py::overload_cast<string, bool>(&DOGI::populateGeneOntologyTerms),
           "obo_file"_a, "overwrite"_a = true)

      .def("populateGeneOntologyAnnotation",
           py::overload_cast<string, bool>(
               &DOGI::populateGeneOntologyAnnotation),
           "gaf_file"_a, "overwrite"_a = true)

      .def("populateStructuralVariants",
           py::overload_cast<string, bool>(&DOGI::populateStructuralVariants),
           "gvf_file"_a, "overwrite"_a = true);
}
