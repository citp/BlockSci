//
//  witness_unknown_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//
//

#include "witness_unknown_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_witness_unknown(py::class_<script::WitnessUnknown> &cl) {
    cl
    .def("__repr__", &script::WitnessUnknown::toString)
    .def("__str__", &script::WitnessUnknown::toPrettyString)
    ;
}

void addWitnessUnknownRangeMethods(RangeClasses<script::WitnessUnknown> &classes) {
	addAllRangeMethods(classes);
}
