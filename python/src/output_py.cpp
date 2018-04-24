//
//  output_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "output_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_output(py::module &m) {
    py::class_<Output> outputClass(m, "Output", "Class representing a transaction output");
    outputClass
    .def("__repr__", &Output::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Output::getAccess, py::return_value_policy::reference)
    ;
    
    addOutputMethods(outputClass, [](auto func) {
        return applyMethodsToSelf<Output>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
}
