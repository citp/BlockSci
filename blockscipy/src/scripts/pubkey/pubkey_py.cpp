//
//  pubkey_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "pubkey_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

using namespace blocksci;
namespace py = pybind11;

void init_pubkey(py::class_<script::Pubkey> &cl) {
    cl
    .def("__repr__", &script::Pubkey::toString)
    .def("__str__", &script::Pubkey::toPrettyString)
    ;
}

void init_pubkeyhash(py::class_<script::PubkeyHash> &cl) {
    cl
    .def("__repr__", &script::PubkeyHash::toString)
    .def("__str__", &script::PubkeyHash::toPrettyString)
    ;
}

void init_witness_pubkeyhash(py::class_<script::WitnessPubkeyHash> &cl) {
    cl
    .def("__repr__", &script::WitnessPubkeyHash::toString)
    .def("__str__", &script::WitnessPubkeyHash::toPrettyString)
    ;
}

void init_multisig_pubkey(py::class_<script::MultisigPubkey> &cl) {    
    cl
    .def("__repr__", &script::MultisigPubkey::toString)
    .def("__str__", &script::MultisigPubkey::toPrettyString)
    ;
}
