//
//  multisig_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "multisig_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_multisig(py::class_<script::Multisig> &cl) {
    cl
    .def("__repr__", &script::Multisig::toString)
    .def("__str__", &script::Multisig::toPrettyString)
    ;
}

void addMultisigRangeMethods(RangeClasses<script::Multisig> &classes) {
	addAllRangeMethods(classes);
}
