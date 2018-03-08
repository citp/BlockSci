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
#include <blocksci/address/dedup_address.hpp>
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
    .def(py::init<uint32_t, AddressType::Enum>(), "Can be constructed directly by passing it an address index and address type")
    .def("__repr__", &Address::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("script_num", &Address::scriptNum, "The internal identifier of the address")
    .def_readonly("type", &Address::type, "The type of address")
    .def_property_readonly("dedup", &Address::dedup, "Returns the deduped address associated with this address")
    .def("balance", py::overload_cast<BlockHeight>(&Address::calculateBalance, py::const_), py::arg("height") = 0, "Calculates the balance held by this address at the height (Defaults to the full chain)")
    .def("outs", py::overload_cast<>(&Address::getOutputs, py::const_), "Returns a list of all outputs sent to this address")
    .def("ins", py::overload_cast<>(&Address::getInputs, py::const_), "Returns a list of all inputs spent from this address")
    .def("txes", py::overload_cast<>(&Address::getTransactions, py::const_), "Returns a list of all transactions involving this address")
    .def("in_txes", py::overload_cast<>(&Address::getInputTransactions, py::const_), "Returns a list of all transaction where this address was an input")
    .def("out_txes", py::overload_cast<>(&Address::getOutputTransactions, py::const_), "Returns a list of all transaction where this address was an output")
    .def("out_txes_count", [](const Address &address) {
        return address.getOutputTransactions().size();
    }, "Return the number of transactions where this address was an output")
    .def("in_txes_count", [](const Address &address) {
        return address.getInputTransactions().size();
    }, "Return the number of transactions where this address was an input")
    .def_property_readonly("script", [](const Address &address) {
        return address.getScript().wrapped;
    }, "Returns the script associated with this address")
    .def_static("address_count", static_cast<size_t(*)()>(addressCount), "Get the total number of address of a given type")
    .def_static("from_string", py::overload_cast<const std::string &>(getAddressFromString), "Construct an address object from an address string")
    .def_static("with_prefix", py::overload_cast<const std::string &>(getAddressesWithPrefix), "Find all addresses beginning with the given prefix")
    ;
    
    py::class_<DedupAddress>(m, "DedupAddress", "Class representing a deduped address which coins are sent to")
    .def(py::init<uint32_t, DedupAddressType::Enum>(), "Can be constructed directly by passing it an script index and script type")
    .def("__repr__", &DedupAddress::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("script_num", &DedupAddress::scriptNum, "The internal identifier of the address")
    .def_readonly("type", &DedupAddress::type, "The kind of script")
    .def("balance", py::overload_cast<BlockHeight>(&DedupAddress::calculateBalance, py::const_), py::arg("height") = 0, "Calculates the balance held by this deduped address at the height (Defaults to the full chain)")
    .def("outs", py::overload_cast<>(&DedupAddress::getOutputs, py::const_), "Returns a list of all outputs sent to this deduped address")
    .def("ins", py::overload_cast<>(&DedupAddress::getInputs, py::const_), "Returns a list of all inputs spent from this deduped address")
    .def("txes", py::overload_cast<>(&DedupAddress::getTransactions, py::const_), "Returns a list of all transactions involving this deduped address")
    .def("in_txes", py::overload_cast<>(&DedupAddress::getInputTransactions, py::const_), "Returns a list of all transaction where this deduped address was an input")
    .def("out_txes", py::overload_cast<>(&DedupAddress::getOutputTransactions, py::const_), "Returns a list of all transaction where this deduped address was an output")
    ;
    
    py::class_<Script> script(m, "Script", "Class representing a script which coins are sent to");
    script
    .def_property_readonly("has_been_spent", py::overload_cast<>(&Script::hasBeenSpent, py::const_), "Check if this script has ever been spent")
    .def_property_readonly("first_tx", py::overload_cast<>(&Script::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_property_readonly("revealed_tx",py::overload_cast<>(&Script::getTransactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    ;
    
    py::class_<PubkeyAddressBase> pubkeyAddressBase(m, "PubkeyAddressBase", script, "Extra data about pay to pubkey address");
    pubkeyAddressBase
    .def_property_readonly("pubkey", [](const PubkeyAddressBase &script) -> ranges::optional<py::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return py::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }, pybind11::keep_alive<0, 1>(), "Public key for this address")
    .def_readonly("pubkeyhash", &PubkeyAddressBase::pubkeyhash, "160 bit address hash")
    ;
    
    py::class_<script::Pubkey>(m, "PubkeyScript", pubkeyAddressBase, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::Pubkey::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Pubkey::toPrettyString, py::const_))
    .def_property_readonly("address_string", py::overload_cast<>(&script::Pubkey::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::PubkeyHash>(m, "PubkeyHashScript", pubkeyAddressBase, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::PubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::PubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("address_string", py::overload_cast<>(&script::PubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessPubkeyHash>(m, "WitnessPubkeyHashScript", pubkeyAddressBase, "Extra data about pay to pubkey address")
    .def("__repr__", py::overload_cast<>(&script::WitnessPubkeyHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessPubkeyHash::toPrettyString, py::const_))
    .def_property_readonly("address_string", py::overload_cast<>(&script::WitnessPubkeyHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::Multisig>(m, "MultisigScript", script, "Extra data about multi-signature address")
    .def("__repr__", py::overload_cast<>(&script::Multisig::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::Multisig::toPrettyString, py::const_))
    .def_readonly("required", &script::Multisig::required, "The number of signatures required for this address")
    .def_readonly("total", &script::Multisig::total, "The total number of keys that can sign for this address")
    .def_readonly("addresses", &script::Multisig::addresses, "The list of the keys that can sign for this address")
    ;
    
    
    py::class_<ScriptHashBase> scriptHashBase(m, "ScriptHashBase", script, "Extra data about pay to script hash address");
    scriptHashBase
    .def_property_readonly("wrapped_address", &ScriptHashBase::getWrappedAddress, "The address inside this P2SH address")
    .def_property_readonly("wrapped_script", [](const ScriptHashBase &script) -> ranges::optional<AnyScript::ScriptVariant> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript->wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "The script inside this P2SH address")
    ;
    
    
    py::class_<script::ScriptHash>(m, "PayToScriptHashScript", scriptHashBase, "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::ScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::ScriptHash::toPrettyString, py::const_))
    .def_readonly("raw_address",  &script::ScriptHash::address, "The 160 bit P2SH address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::ScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::WitnessScriptHash>(m, "PayToWitnessScriptHashScript", scriptHashBase, "Extra data about pay to script hash address")
    .def("__repr__", py::overload_cast<>(&script::WitnessScriptHash::toString, py::const_))
    .def("__str__", py::overload_cast<>(&script::WitnessScriptHash::toPrettyString, py::const_))
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
    
    py::enum_<DedupAddressType::Enum>(m, "dedup_address_type", py::arithmetic(), "Enumeration of all dedup address types")
    .value("nonstandard", DedupAddressType::NONSTANDARD)
    .value("pubkey", DedupAddressType::PUBKEY)
    .value("scripthash", DedupAddressType::SCRIPTHASH)
    .value("multisig", DedupAddressType::MULTISIG)
    .value("nulldata", DedupAddressType::NULL_DATA)
    .def_property_readonly_static("types", [](py::object) -> std::array<DedupAddressType::Enum, 5> {
        return {{DedupAddressType::PUBKEY, DedupAddressType::SCRIPTHASH,
            DedupAddressType::MULTISIG, DedupAddressType::NULL_DATA, DedupAddressType::NONSTANDARD}};
    }, "A list of all possible dedup address types")
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
