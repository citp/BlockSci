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
#include <blocksci/address/equiv_address.hpp>
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
    .def_property_readonly("equiv", &Address::equiv, "Returns the EquivAddress associated with this address")
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
    
    py::class_<EquivAddress>(m, "EquivAddress", "Class representing a equivalent address which coins are sent to")
    .def(py::init<uint32_t, DedupAddressType::Enum>(), "Can be constructed directly by passing it an script index and script type")
    .def("__repr__", &EquivAddress::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("script_num", &EquivAddress::scriptNum, "The internal identifier of the address")
    .def_readonly("type", &EquivAddress::type, "The kind of script")
    ;
    
    py::class_<Script> script(m, "Script", "Class representing a script which coins are sent to");
    script
    .def_property_readonly("has_been_spent", py::overload_cast<>(&Script::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&Script::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&Script::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    ;
    
    py::class_<script::Pubkey>(m, "PubkeyScript", script, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::Pubkey::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Pubkey::toPrettyString, py::const_))
    .def_property_readonly("pubkey", [](const script::Pubkey &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_readonly("pubkeyhash", &script::Pubkey::pubkeyhash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::Pubkey::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::PubkeyHash>(m, "PubkeyHashScript", script, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::PubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::PubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("pubkey", [](const script::PubkeyHash &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_readonly("pubkeyhash", &script::PubkeyHash::pubkeyhash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::PubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessPubkeyHash>(m, "WitnessPubkeyHashScript", script, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::WitnessPubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessPubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("pubkey", [](const script::WitnessPubkeyHash &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_readonly("pubkeyhash", &script::WitnessPubkeyHash::pubkeyhash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::WitnessPubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::Multisig>(m, "MultisigScript", script, "Extra data about multi-signature address")
    .def("__repr__", py::overload_cast<>(&script::Multisig::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Multisig::toPrettyString, py::const_))
    .def_readonly("required", &script::Multisig::required, "The number of signatures required for this address")
    .def_readonly("total", &script::Multisig::total, "The total number of keys that can sign for this address")
    .def_readonly("addresses", &script::Multisig::addresses, "The list of the keys that can sign for this address")
    ;
    
    
    py::class_<script::ScriptHash>(m, "PayToScriptHashScript", script, "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::ScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::ScriptHash::toPrettyString, py::const_))
    .def_property_readonly("wrapped_address", &script::ScriptHash::getWrappedAddress, "The address inside this P2SH address")
    .def_property_readonly("wrapped_script", [](const script::ScriptHash &script) -> ranges::optional<AnyScript::ScriptVariant> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript->wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "The script inside this P2SH address")
    .def_readonly("raw_address",  &script::ScriptHash::address, "The 160 bit P2SH address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::ScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessScriptHash>(m, "PayToWitnessScriptHashScript", script, "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::WitnessScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessScriptHash::toPrettyString, py::const_))
    .def_property_readonly("wrapped_address", &script::WitnessScriptHash::getWrappedAddress, "The address inside this witness script hash address")
    .def_property_readonly("wrapped_script", [](const script::WitnessScriptHash &script) -> ranges::optional<AnyScript::ScriptVariant> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript->wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "The script inside this witness script hash address")
    .def_readonly("raw_address",  &script::WitnessScriptHash::address, "The 256 bit Witness P2SH address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::WitnessScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::OpReturn>(m, "NulldataScript", script, "Extra data about op_return address")
    .def("__repr__", py::overload_cast<>(&script::OpReturn::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::OpReturn::toPrettyString, py::const_))
    .def_property_readonly("data", [](const script::OpReturn &address) {
        return py::bytes(address.data);
    }, pybind11::keep_alive<0, 1>(), "Data contained inside this script")
    ;
    
    py::class_<script::Nonstandard>(m, "NonStandardScript", script, "Extra data about non-standard address")
    .def("__repr__", py::overload_cast<>(&script::Nonstandard::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Nonstandard::toPrettyString, py::const_))
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
    
    py::enum_<DedupAddressType::Enum>(m, "equiv_address_type", py::arithmetic(), "Enumeration of all equiv address types")
    .value("nonstandard", DedupAddressType::NONSTANDARD)
    .value("pubkey", DedupAddressType::PUBKEY)
    .value("scripthash", DedupAddressType::SCRIPTHASH)
    .value("multisig", DedupAddressType::MULTISIG)
    .value("nulldata", DedupAddressType::NULL_DATA)
    .def_property_readonly_static("types", [](py::object) -> std::array<DedupAddressType::Enum, 5> {
        return {{DedupAddressType::PUBKEY, DedupAddressType::SCRIPTHASH,
            DedupAddressType::MULTISIG, DedupAddressType::NULL_DATA, DedupAddressType::NONSTANDARD}};
    }, "A list of all possible equiv address types")
    .def("__str__", [](DedupAddressType::Enum val) {
        switch (val) {
            case DedupAddressType::PUBKEY:
                return "Pay to pubkey";
            case DedupAddressType::SCRIPTHASH:
                return "Pay to script hash";
            case DedupAddressType::MULTISIG:
                return "Multisig";
            case DedupAddressType::NONSTANDARD:
                return "Nonstandard";
            case DedupAddressType::NULL_DATA:
                return "Null data";
            default:
                return "Unknown Address Type";
        }
    })
    ;

}
