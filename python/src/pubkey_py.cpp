//
//  pubkey_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "pubkey_py.hpp"
#include "address_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

using namespace blocksci;
namespace py = pybind11;

const char *pubkeyDocstring(std::string docstring) {
    return strdup(docstring.c_str());
}

void init_pubkey(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Pubkey> pubkeyAddressCl(m, "PubkeyAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyAddressCl
    .def("__repr__", &script::Pubkey::toString)
    .def("__str__", &script::Pubkey::toPrettyString)
    ;

    applyMethodsToSelf(pubkeyAddressCl, AddPubkeyBaseMethods<script::Pubkey>{pubkeyDocstring});
    
    py::class_<script::PubkeyHash> pubkeyHashAddressCl(m, "PubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyHashAddressCl
    .def("__repr__", &script::PubkeyHash::toString)
    .def("__str__", &script::PubkeyHash::toPrettyString)
    ;

    applyMethodsToSelf(pubkeyHashAddressCl, AddPubkeyBaseMethods<script::PubkeyHash>{pubkeyDocstring});

    py::class_<script::WitnessPubkeyHash> witnessPubkeyHashAddressCl(m, "WitnessPubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    witnessPubkeyHashAddressCl
    .def("__repr__", &script::WitnessPubkeyHash::toString)
    .def("__str__", &script::WitnessPubkeyHash::toPrettyString)
    ;

    applyMethodsToSelf(witnessPubkeyHashAddressCl, AddPubkeyBaseMethods<script::WitnessPubkeyHash>{pubkeyDocstring});
    
    py::class_<script::MultisigPubkey> multisigPubkeyCl(m, "MultisigPubkey", addressCl, "Extra data about a pubkey inside a multisig address");
    multisigPubkeyCl
    .def("__repr__", &script::MultisigPubkey::toString)
    .def("__str__", &script::MultisigPubkey::toPrettyString)
    ;

    applyMethodsToSelf(multisigPubkeyCl, AddPubkeyBaseMethods<script::MultisigPubkey>{pubkeyDocstring});
}
