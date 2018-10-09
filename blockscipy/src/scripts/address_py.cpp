//
//  address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/access.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/operators.h>

#include <range/v3/view/transform.hpp>

namespace py = pybind11;

using namespace blocksci;

void init_address_type(py::module &m) {
    py::enum_<AddressType::Enum>(m, "address_type", py::arithmetic(), "Enumeration of all address types")
    .value("nonstandard", AddressType::Enum::NONSTANDARD)
    .value("pubkey", AddressType::Enum::PUBKEY)
    .value("pubkeyhash", AddressType::Enum::PUBKEYHASH)
    .value("multisig_pubkey", AddressType::Enum::MULTISIG_PUBKEY)
    .value("scripthash", AddressType::Enum::SCRIPTHASH)
    .value("multisig", AddressType::Enum::MULTISIG)
    .value("nulldata", AddressType::Enum::NULL_DATA)
    .value("witness_pubkeyhash", AddressType::Enum::WITNESS_PUBKEYHASH)
    .value("witness_scripthash", AddressType::Enum::WITNESS_SCRIPTHASH)
    .def_property_readonly_static("types", [](py::object) -> std::array<AddressType::Enum, 9> {
        return {{AddressType::Enum::NONSTANDARD, AddressType::Enum::PUBKEY, AddressType::Enum::PUBKEYHASH, 
            AddressType::Enum::MULTISIG_PUBKEY, AddressType::Enum::SCRIPTHASH,
            AddressType::Enum::MULTISIG, AddressType::Enum::NULL_DATA,
            AddressType::Enum::WITNESS_PUBKEYHASH, AddressType::Enum::WITNESS_SCRIPTHASH}};
        }, "A list of all possible address types")
    .def("__str__", [](AddressType::Enum val) {
        switch (val) {
            case AddressType::Enum::PUBKEY:
                return "Pay to pubkey";
            case AddressType::Enum::PUBKEYHASH:
                return "Pay to pubkey hash";
            case AddressType::Enum::SCRIPTHASH:
                return "Pay to script hash";
            case AddressType::Enum::MULTISIG:
                return "Multisig";
            case AddressType::Enum::MULTISIG_PUBKEY:
                return "Multisig Public Key";
            case AddressType::Enum::NONSTANDARD:
                return "Nonstandard";
            case AddressType::Enum::NULL_DATA:
                return "Null data";
            case AddressType::Enum::WITNESS_PUBKEYHASH:
                return "Pay to witness pubkey hash";
            case AddressType::Enum::WITNESS_SCRIPTHASH:
                return "Pay to witness script hash";
            default:
                return "Unknown Address Type";
        }
    })
    ;
}

void init_address(py::class_<blocksci::ScriptBase> &cl) {
    cl
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", [](const ScriptBase &script) {
        return Access{&script.getAccess()};
    })
    .def("ins", &ScriptBase::getInputs, "Returns a list of all inputs spent from this address")
    .def("txes", &ScriptBase::getTransactions, "Returns a list of all transactions involving this address")
    .def("in_txes",&ScriptBase::getInputTransactions, "Returns a list of all transaction where this address was an input")
    .def("out_txes", &ScriptBase::getOutputTransactions, "Returns a list of all transaction where this address was an output")
    ;
}

void addAddressRangeMethods(RangeClasses<AnyScript> &classes) {
    addAllRangeMethods(classes);
}
