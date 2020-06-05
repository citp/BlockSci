//
//  scripthash_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "scripthash_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/scripts/scripthash_script.hpp>

using namespace blocksci;
namespace py = pybind11;

void init_scripthash(py::class_<script::ScriptHash> &cl) {
    cl
    .def("__repr__", &script::ScriptHash::toString)
    .def("__str__", &script::ScriptHash::toPrettyString)
    ;
}

void init_witness_scripthash(py::class_<script::WitnessScriptHash> &cl) {
    
    cl
    .def("__repr__", &script::WitnessScriptHash::toString)
    .def("__str__", &script::WitnessScriptHash::toPrettyString)
    ;
}

void addScriptHashRangeMethods(RangeClasses<script::ScriptHash> &classes) {
	addAllRangeMethods(classes);
}

void addWitnessScriptHashRangeMethods(RangeClasses<script::WitnessScriptHash> &classes) {
	addAllRangeMethods(classes);
}
