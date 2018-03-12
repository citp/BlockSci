//
//  address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"
#include "variant_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/chain.hpp>
#include <blocksci/script.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_address(py::module &m) {
    py::class_<Address> address(m, "Address", "Represents an abstract address object which uniquely identifies a given address");
    address
    .def(py::init<uint32_t, AddressType::Enum, const DataAccess &>(), "Can be constructed directly by passing it an address index and address type")
    .def("__repr__", &Address::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("script_num", &Address::scriptNum, "The internal identifier of the address")
    .def_readonly("type", &Address::type, "The type of address")
    .def("balance", &Address::calculateBalance, py::arg("height") = 0, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Calculates the balance held by this address at the height (Defaults to the full chain)")
    .def("outs", &Address::getOutputs, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Returns a list of all outputs sent to this address")
    .def("ins", &Address::getInputs, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Returns a list of all inputs spent from this address")
    .def("txes", &Address::getTransactions, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Returns a list of all transactions involving this address")
    .def("in_txes",&Address::getInputTransactions, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Returns a list of all transaction where this address was an input")
    .def("out_txes", &Address::getOutputTransactions, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Returns a list of all transaction where this address was an output")
    .def("out_txes_count", [](const Address &address, bool typeEquivalent, bool nestedEquivalent) {
        return address.getOutputTransactions(typeEquivalent, nestedEquivalent).size();
    }, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Return the number of transactions where this address was an output")
    .def("in_txes_count", [](const Address &address, bool typeEquivalent, bool nestedEquivalent) {
        return address.getInputTransactions(typeEquivalent, nestedEquivalent).size();
    }, py::arg("typeEquivalent") = true,  py::arg("nestedEquivalent") = true, "Return the number of transactions where this address was an input")
    .def_property_readonly("script", [](const Address &address) {
        return address.getScript().wrapped;
    }, "Returns the script associated with this address")
    ;
    
    py::class_<script::Pubkey>(m, "PubkeyScript", "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::Pubkey::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Pubkey::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::Pubkey::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::Pubkey::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::Pubkey::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("pubkey", [](const script::Pubkey &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_property_readonly("pubkeyhash", &script::Pubkey::getPubkeyHash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::Pubkey::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::PubkeyHash>(m, "PubkeyHashScript", "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::PubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::PubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::PubkeyHash::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::PubkeyHash::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::PubkeyHash::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("pubkey", [](const script::PubkeyHash &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_property_readonly("pubkeyhash", &script::PubkeyHash::getPubkeyHash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::PubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessPubkeyHash>(m, "WitnessPubkeyHashScript", "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::WitnessPubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessPubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::WitnessPubkeyHash::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::WitnessPubkeyHash::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::WitnessPubkeyHash::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("pubkey", [](const script::WitnessPubkeyHash &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_property_readonly("pubkeyhash", &script::WitnessPubkeyHash::getPubkeyHash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::WitnessPubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::Multisig>(m, "MultisigScript", "Extra data about multi-signature address")
    .def("__repr__", py::overload_cast<>(&script::Multisig::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Multisig::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::Multisig::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::Multisig::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::Multisig::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("required", &script::Multisig::getRequired, "The number of signatures required for this address")
    .def_property_readonly("total", &script::Multisig::getTotal, "The total number of keys that can sign for this address")
    .def_property_readonly("addresses", &script::Multisig::getAddresses, "The list of the keys that can sign for this address")
    ;
    
    
    py::class_<script::ScriptHash>(m, "PayToScriptHashScript", "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::ScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::ScriptHash::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::ScriptHash::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::ScriptHash::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::ScriptHash::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("wrapped_address", &script::ScriptHash::getWrappedAddress, "The address inside this P2SH address")
    .def_property_readonly("wrapped_script", [](const script::ScriptHash &script) -> ranges::optional<AnyScript::ScriptVariant> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript->wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "The script inside this P2SH address")
    .def_property_readonly("raw_address",  &script::ScriptHash::getAddressHash, "The 160 bit P2SH address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::ScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessScriptHash>(m, "PayToWitnessScriptHashScript", "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::WitnessScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessScriptHash::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::WitnessScriptHash::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::WitnessScriptHash::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::WitnessScriptHash::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("wrapped_address", &script::WitnessScriptHash::getWrappedAddress, "The address inside this witness script hash address")
    .def_property_readonly("wrapped_script", [](const script::WitnessScriptHash &script) -> ranges::optional<AnyScript::ScriptVariant> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript->wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "The script inside this witness script hash address")
    .def_property_readonly("raw_address",  &script::WitnessScriptHash::getAddressHash, "The 256 bit Witness P2SH address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::WitnessScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::OpReturn>(m, "NulldataScript", "Extra data about op_return address")
    .def("__repr__", py::overload_cast<>(&script::OpReturn::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::OpReturn::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::OpReturn::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::OpReturn::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::OpReturn::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("data", [](const script::OpReturn &address) {
        return py::bytes(address.getData());
    }, pybind11::keep_alive<0, 1>(), "Data contained inside this script")
    ;
    
    py::class_<script::Nonstandard>(m, "NonStandardScript", "Extra data about non-standard address")
    .def("__repr__", py::overload_cast<>(&script::Nonstandard::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Nonstandard::toPrettyString, py::const_))
    .def_property_readonly("has_been_spent", py::overload_cast<>(&script::Nonstandard::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&script::Nonstandard::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&script::Nonstandard::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    .def_property_readonly("in_script", &script::Nonstandard::inputString, "Nonstandard input script")
    .def_property_readonly("out_script", &script::Nonstandard::outputString, "Nonstandard output script")
    ;
    
    py::enum_<AddressType::Enum>(m, "address_type", py::arithmetic(), "Enumeration of all address types")
    .value("nonstandard", AddressType::Enum::NONSTANDARD)
    .value("pubkey", AddressType::Enum::PUBKEY)
    .value("pubkeyhash", AddressType::Enum::PUBKEYHASH)
    .value("scripthash", AddressType::Enum::SCRIPTHASH)
    .value("multisig", AddressType::Enum::MULTISIG)
    .value("nulldata", AddressType::Enum::NULL_DATA)
    .value("witness_pubkeyhash", AddressType::Enum::WITNESS_PUBKEYHASH)
    .value("witness_scripthash", AddressType::Enum::WITNESS_SCRIPTHASH)
    .def_property_readonly_static("types", [](py::object) -> std::array<AddressType::Enum, 8> {
        return {{AddressType::Enum::PUBKEY, AddressType::Enum::PUBKEYHASH, AddressType::Enum::SCRIPTHASH,
            AddressType::Enum::MULTISIG, AddressType::Enum::NULL_DATA, AddressType::Enum::NONSTANDARD,
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
