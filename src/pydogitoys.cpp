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
      .def(py::init<const string&, const string&>(), "path"_a, "config"_a = "")
      .def("open", py::overload_cast<const string&, bool>(&DOGI::open),
           "path"_a, "create"_a = false)
      .def("close", &DOGI::close, "optimize"_a = false)
      //        .def("populate", py::overload_cast<const
      //        string&>(&DOGI::populate), "config"_a) .def("close",
      //        &DOGI::close)
      ;
}
