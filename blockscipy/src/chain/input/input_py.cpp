//
//  input_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "input_py.hpp"
#include "input_properties_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/chain/access.hpp>
#include <blocksci/chain/block.hpp>

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_input(py::class_<Input> &cl) {
    applyMethodsToSelf(cl, AddInputMethods{});

    cl
    .def("__repr__", &Input::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", [](const Input &input) {
        return Access{&input.getAccess()};
    })
    ;
}

void addInputRangeMethods(RangeClasses<Input> &classes) {
    addAllRangeMethods(classes);
}
