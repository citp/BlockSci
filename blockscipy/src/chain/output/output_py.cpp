//
//  output_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "output_py.hpp"
#include "output_properties_py.hpp"
#include "self_apply_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <pybind11/operators.h>

#include <blocksci/chain/access.hpp>
#include <blocksci/chain/block.hpp>

namespace py = pybind11;

using namespace blocksci;

void init_output(py::class_<Output> &cl) {
    applyMethodsToSelf(cl, AddOutputMethods{});

    cl
    .def("__repr__", &Output::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", [](const Output &output) {
        return Access{&output.getAccess()};
    })
    ;
}

void addOutputRangeMethods(RangeClasses<Output> &classes) {
    addAllRangeMethods(classes);
}
