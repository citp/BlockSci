//
//  multisig_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "multisig_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

pybind11::list pyMultisigAddresses(blocksci::script::Multisig &script) {
    pybind11::list ret;
    for (auto &address : script.pubkeyScripts()) {
        ret.append(address);
    }
    return ret;
}

void init_multisig(py::class_<script::Multisig> &cl) {
    cl
    .def("__repr__", &script::Multisig::toString)
    .def("__str__", &script::Multisig::toPrettyString)
    ;
}
