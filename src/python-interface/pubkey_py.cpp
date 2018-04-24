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

void init_pubkey(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Pubkey> pubkeyAddressCl(m, "PubkeyAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyAddressCl
    .def("__repr__", &script::Pubkey::toString)
    .def("__str__", &script::Pubkey::toPrettyString)
    ;

    addPubkeyBaseMethods<script::Pubkey>(pubkeyAddressCl, [](auto func) {
        return applyMethodsToSelf<script::Pubkey>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::PubkeyHash> pubkeyHashAddressCl(m, "PubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyHashAddressCl
    .def("__repr__", &script::PubkeyHash::toString)
    .def("__str__", &script::PubkeyHash::toPrettyString)
    ;

    addPubkeyBaseMethods<script::PubkeyHash>(pubkeyHashAddressCl, [](auto func) {
        return applyMethodsToSelf<script::PubkeyHash>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::WitnessPubkeyHash> witnessPubkeyHashAddressCl(m, "WitnessPubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    witnessPubkeyHashAddressCl
    .def("__repr__", &script::WitnessPubkeyHash::toString)
    .def("__str__", &script::WitnessPubkeyHash::toPrettyString)
    ;

    addPubkeyBaseMethods<script::WitnessPubkeyHash>(witnessPubkeyHashAddressCl, [](auto func) {
        return applyMethodsToSelf<script::WitnessPubkeyHash>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::MultisigPubkey> multisigPubkeyCl(m, "MultisigPubkey", addressCl, "Extra data about a pubkey inside a multisig address");
    multisigPubkeyCl
    .def("__repr__", &script::MultisigPubkey::toString)
    .def("__str__", &script::MultisigPubkey::toPrettyString)
    ;

    addPubkeyBaseMethods<script::MultisigPubkey>(multisigPubkeyCl, [](auto func) {
        return applyMethodsToSelf<script::MultisigPubkey>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
}
