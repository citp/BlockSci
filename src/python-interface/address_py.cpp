//
//  address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include <blocksci/scripts/address_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/address.hpp>
#include <blocksci/scripts/address_types.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace blocksci;

namespace pybind11 { namespace detail {
    template<typename T>
    struct type_caster<boost::optional<T>>
    {
        static handle cast(boost::optional<T> src, return_value_policy policy, handle parent)
        {
            if (src)
            {
                return type_caster<T>::cast(*src, policy, parent);
            }
            
            // if not set convert to None
            return none().inc_ref();
        }
        PYBIND11_TYPE_CASTER(boost::optional<T>, _("Optional"));
    };
}}

void init_address(py::module &m) {
    py::class_<AddressPointer>(m, "AddressPointer", "Pointer to an address")
    .def(py::init<uint32_t, ScriptType::Enum>())
    .def("__repr__", &AddressPointer::toString)
    .def("__eq__", &AddressPointer::operator==)
    .def("__hash__", [] (const AddressPointer &address) {
        uint64_t hash = address.addressNum;
        hash <<= 8;
        hash += static_cast<uint64_t>(address.type);
        return hash;
    })
    .def_readonly("address_num", &AddressPointer::addressNum)
    .def_readonly("type", &AddressPointer::type)
    .def("address", static_cast<std::unique_ptr<Address>(AddressPointer::*)() const>(&AddressPointer::getAddress))
    .def("get_first_tx", static_cast<Transaction(AddressPointer::*)() const>(&AddressPointer::getFirstTransaction), "Get the first transaction that was sent to this address")
    .def_static("from_string", py::overload_cast<const std::string &>(getAddressPointerFromString), "Construct an address object from an address string")
    .def_static("from_strings", py::overload_cast<const std::vector<std::string> &>(getAddressPointersFromStrings))
    .def_static("address_count", static_cast<size_t(*)()>(addressCount))
    .def_static("get_addresses_starting_with_prefix", py::overload_cast<const std::string &>(getAddressPointersStartingWithPrefex))
    ;
    
    py::class_<Address> address(m, "Address", "Class representing an address which coins are sent to");
    address
    .def("__repr__", static_cast<std::string(Address::*)() const>(&Address::toString))
    .def("__str__", static_cast<std::string(Address::*)() const>(&Address::toPrettyString))
    .def("__eq__", &Address::operator==)
    ;
    
    py::class_<address::PubkeyHash>(m, "PubkeyHashData", address, "Extra data about pay to pubkey hash address")
    .def_property_readonly("address", static_cast<std::string(address::PubkeyHash::*)() const>(&address::PubkeyHash::addressString))
    .def_property_readonly("raw_address", [](const address::PubkeyHash &address) {
        return address.address.GetHex();
    })
    ;
    
    py::class_<address::Multisig>(m, "MultisigData", address, "Extra data about multi-signature address")
    .def_readonly("required", &address::Multisig::required)
    .def_readonly("addresses", &address::Multisig::addresses)
    ;
    
    py::class_<address::ScriptHash>(m, "P2SHData", address, "Extra data about pay to script hash address")
    .def_property_readonly("address", static_cast<std::string(address::ScriptHash::*)() const>(&address::ScriptHash::addressString))
    .def_readonly("wrapped_address_pointer", &address::ScriptHash::wrappedAddressPointer)
    .def_property_readonly("raw_address", [](const address::ScriptHash &address) {
        return address.address.GetHex();
    })
    ;
    
    py::class_<address::OpReturn>(m, "NulldataData", address, "Extra data about op_return address")
    .def_property_readonly("data", [](const address::OpReturn &address) {
        return py::bytes(address.data);
    }, py::return_value_policy::copy)
    ;
    
    py::class_<address::Nonstandard>(m, "NonStandardData", address, "Extra data about non-standard address")
    .def_property_readonly("input_script", &address::Nonstandard::inputString)
    .def_property_readonly("output_script", &address::Nonstandard::outputString)
    ;
    
    py::enum_<ScriptType::Enum>(m, "txnouttype", py::arithmetic(), "Enumeration of all transaction output types")
    .value("nonstandard", ScriptType::Enum::NONSTANDARD)
    .value("pubkey", ScriptType::Enum::PUBKEY)
    .value("pubkeyhash", ScriptType::Enum::PUBKEYHASH)
    .value("scripthash", ScriptType::Enum::SCRIPTHASH)
    .value("multisig", ScriptType::Enum::MULTISIG)
    .value("nulldata", ScriptType::Enum::NULL_DATA)
    .export_values()
    ;

}
