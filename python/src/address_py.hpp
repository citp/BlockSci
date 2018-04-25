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
int64_t pyAddressAddressNum(T &address) {
    return address.getScriptNum();
}

template <typename T>
blocksci::AddressType::Enum pyAddressType(T &address) {
    return address.getType();
}

template <typename T>
blocksci::EquivAddress pyAddressEquiv(T &address, bool equiv_script) {
    return address.getEquivAddresses(equiv_script);
}

template <typename T>
int64_t pyAddressBalance(T &address, int height) {
    return address.calculateBalance(height);
}

template <typename T>
int64_t pyAddressOutTxesCount(T &address) {
    return ranges::distance(address.getOutputTransactions());
}

template <typename T>
int64_t pyAddressInTxesCount(T &address) {
    return ranges::distance(address.getInputTransactions());
}

template <typename T>
blocksci::Transaction pyAddressFirstTx(T &address) {
    return address.getFirstTransaction();
}

template <typename T>
ranges::optional<blocksci::Transaction> pyAddressRevealedTx(T &address) {
    return address.getTransactionRevealed();
}

template <typename T>
bool pyAddressHasBeenSpent(T &address) {
    return address.hasBeenSpent();
}

template <typename T>
ranges::any_view<blocksci::Output> pyAddressOuts(T &address) {
    return address.getOutputs();
}

template <typename T, typename Class, typename FuncApplication, typename FuncDoc>
void addAddressMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address_num", func(pyAddressAddressNum<T>), func2("The internal identifier of the address"))
    .def_property_readonly("type", func(pyAddressType<T>), func2("The type of address"))
    .def("equiv", func(pyAddressEquiv<T>), pybind11::arg("equiv_script") = true, "Returns a list of all addresses equivalent to this address")
    .def("balance", func(pyAddressBalance<T>), pybind11::arg("height") = -1, func2("Calculates the balance held by this address at the height (Defaults to the full chain)"))
    .def("out_txes_count", func(pyAddressOutTxesCount<T>), func2("Return the number of transactions where this address was an output"))
    .def("in_txes_count", func(pyAddressInTxesCount<T>), func2("Return the number of transactions where this address was an input"))
    .def_property_readonly("first_tx", func(pyAddressFirstTx<T>), func2("Get the first transaction that was sent to a type equivalent address"))
    .def_property_readonly("revealed_tx", func(pyAddressRevealedTx<T>), func2("The transaction where a type equivalent address was first revealed"))
    .def_property_readonly("has_been_spent", func(pyAddressHasBeenSpent<T>), func2("Check if a type equivalent address has ever been spent"))
    .def_property_readonly("outs", func(pyAddressOuts<T>), func2("Returns a range of all outputs sent to this address"))
    ;
}

#endif /* blocksci_address_py_h */
