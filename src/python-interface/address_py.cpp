//
//  address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "address_py.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/transaction.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <range/v3/view/transform.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename T, typename T2>
auto addAddressRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addAddressMethods<T2>(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each address: " << docstring;
        return strdup(ss.str().c_str());
    });
    applyRangeMethodsToRange<AddAddressRangeMethods>(cl);
    return cl;
}

template <typename T, typename T2>
auto addOptionalAddressRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addAddressMethods<T2>(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each address: " << docstring;
        return strdup(ss.str().c_str());
    });
    applyRangeMethodsToRange<AddAddressRangeMethods>(cl);
    return cl;
}

py::class_<ScriptBase> init_address(py::module &m) {
    py::class_<ScriptBase> addressCl(m, "Address", "Represents an abstract address object which uniquely identifies a given address");
    addressCl
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &ScriptBase::getAccess, py::return_value_policy::reference)
    .def("ins", &ScriptBase::getInputs, "Returns a list of all inputs spent from this address")
    .def("txes", &ScriptBase::getTransactions, "Returns a list of all transactions involving this address")
    .def("in_txes",&ScriptBase::getInputTransactions, "Returns a list of all transaction where this address was an input")
    .def("out_txes", &ScriptBase::getOutputTransactions, "Returns a list of all transaction where this address was an output")
    ;

    addAddressMethods<ScriptBase>(addressCl, [](auto func) {
        return applyMethodsToSelf<ScriptBase>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<EquivAddress>(m, "EquivAddress", "A set of equivalent addresses")
    .def(py::self == py::self)
    .def(hash(py::self))
    .def("__repr__", &EquivAddress::toString)
    .def("__len__", [](const EquivAddress &address) { return address.size(); })
    .def("__bool__", [](const EquivAddress &address) { return address.size() == 0; })
    .def("__iter__", [](const EquivAddress &address) {
        auto transformed = address | ranges::view::transform([](const Address &address) {
            return address.getScript();
        });
        return py::make_iterator(transformed.begin(), transformed.end());
    },py::keep_alive<0, 1>())
    .def_property_readonly("is_script_equiv", &EquivAddress::isScriptEquiv, "Returns whether this equiv address is script equivalent or not")
    .def("balance", &EquivAddress::calculateBalance, py::arg("height") = -1, "Calculates the balance held by these equivalent addresses at the height (Defaults to the full chain)")
    .def("outs", &EquivAddress::getOutputs, "Returns a list of all outputs sent to these equivalent addresses")
    .def("ins", &EquivAddress::getInputs, "Returns a list of all inputs spent from these equivalent addresses")
    .def("txes", &EquivAddress::getTransactions, "Returns a list of all transactions involving these equivalent addresses")
    .def("out_txes",&EquivAddress::getOutputTransactions, "Returns a range of all transaction where these equivalent addresses were an output")
    .def("in_txes",&EquivAddress::getInputTransactions, "Returns a list of all transaction where these equivalent addresses were an input")
    .def("out_txes_count", [](EquivAddress &address) {
        return address.getOutputTransactions().size();
    }, "Return the number of transactions where these equivalent addresses were an output")
    .def("in_txes_count", [](EquivAddress &address) {
        return address.getInputTransactions().size();
    }, "Return the number of transactions where these equivalent addresses were an input")
    ;

    addAddressRange<ranges::any_view<AnyScript>, AnyScript>(m, "AnyAddressRange");
    addAddressRange<ranges::any_view<AnyScript, ranges::category::random_access>, AnyScript>(m, "AddressRange");
    addOptionalAddressRange<ranges::any_view<ranges::optional<AnyScript>>, AnyScript>(m, "AnyOptionalAddressRange");
    addOptionalAddressRange<ranges::any_view<ranges::optional<AnyScript>, ranges::category::random_access>, AnyScript>(m, "OptionalAddressRange");
    
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
        return {{AddressType::Enum::PUBKEY, AddressType::Enum::PUBKEYHASH, AddressType::Enum::MULTISIG_PUBKEY, AddressType::Enum::SCRIPTHASH,
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
    return addressCl;
}
