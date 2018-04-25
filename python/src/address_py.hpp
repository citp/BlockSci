//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_address_py_h
#define blocksci_address_py_h

#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <pybind11/pybind11.h>

pybind11::class_<blocksci::ScriptBase> init_address(pybind11::module &m);

template <typename T>
inline int64_t pyAddressOutTxesCount(T &address) {
    return ranges::distance(address.getOutputTransactions());
}

template <typename T>
inline int64_t pyAddressInTxesCount(T &address) {
    return ranges::distance(address.getInputTransactions());
}

template <typename T>
struct AddAddressMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddAddressMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def_property_readonly("address_num", func(&T::getScriptNum), func2("The internal identifier of the address"))
        .def_property_readonly("type", func(&T::getType), func2("The type of address"))
        .def("equiv", func(&T::getEquivAddresses), pybind11::arg("equiv_script") = true, "Returns a list of all addresses equivalent to this address")
        .def("balance", func(&T::calculateBalance), pybind11::arg("height") = -1, func2("Calculates the balance held by this address at the height (Defaults to the full chain)"))
        .def("out_txes_count", func(pyAddressOutTxesCount<T>), func2("Return the number of transactions where this address was an output"))
        .def("in_txes_count", func(pyAddressInTxesCount<T>), func2("Return the number of transactions where this address was an input"))
        .def_property_readonly("first_tx", func(&T::getFirstTransaction), func2("Get the first transaction that was sent to a type equivalent address"))
        .def_property_readonly("revealed_tx", func(&T::getTransactionRevealed), func2("The transaction where a type equivalent address was first revealed"))
        .def_property_readonly("has_been_spent", func(&T::hasBeenSpent), func2("Check if a type equivalent address has ever been spent"))
        .def_property_readonly("outs", func(&T::getOutputs), func2("Returns a range of all outputs sent to this address"))
        ;
    }
};

#endif /* blocksci_address_py_h */
