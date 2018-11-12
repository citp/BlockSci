//
//  nonstandard_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nonstandard_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/scripts/nonstandard_script.hpp>

using namespace blocksci;
namespace py = pybind11;

void init_nonstandard(py::class_<script::Nonstandard> &cl) {
    cl
    .def("__repr__", &script::Nonstandard::toString)
    .def("__str__", &script::Nonstandard::toPrettyString)
    ;
}

void addNonstandardRangeMethods(RangeClasses<script::Nonstandard> &classes) {
	addAllRangeMethods(classes);
}
