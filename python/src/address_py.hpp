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

template <typename T, typename Class, typename FuncApplication, typename FuncDoc>
void addAddressMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address_num", func([](T &address) -> int64_t {
    	return address.getScriptNum();
    }), func2("The internal identifier of the address"))
    .def_property_readonly("type", func([](const T &address) -> AddressType::Enum {
    	return address.getType();
    }), func2("The type of address"))
    .def("equiv", func([](const T &address, bool equiv_script) -> EquivAddress {
    	return address.getEquivAddresses(equiv_script);
    }), pybind11::arg("equiv_script") = true, "Returns a list of all addresses equivalent to this address")
    .def("balance", func([](T &address, int height) -> int64_t {
    	return address.calculateBalance(height);
    }), pybind11::arg("height") = -1, func2("Calculates the balance held by this address at the height (Defaults to the full chain)"))
    .def("out_txes_count", func([](T &address) -> int64_t {
        return ranges::distance(address.getOutputTransactions());
    }), func2("Return the number of transactions where this address was an output"))
    .def("in_txes_count", func([](T &address) -> int64_t {
        return address.getInputTransactions().size();
    }), func2("Return the number of transactions where this address was an input"))
    .def_property_readonly("first_tx", func([](const T &address) -> Transaction {
    	return address.getFirstTransaction();
    }), func2("Get the first transaction that was sent to a type equivalent address"))
    .def_property_readonly("revealed_tx", func([](const T &address) -> ranges::optional<Transaction> {
    	return address.getTransactionRevealed();
    }), func2("The transaction where a type equivalent address was first revealed"))
    .def_property_readonly("has_been_spent", func([](const T &address) -> bool {
    	return address.hasBeenSpent();
    }), func2("Check if a type equivalent address has ever been spent"))
    .def_property_readonly("outs", func([](T &address) -> ranges::any_view<Output> {
        return address.getOutputs();
    }), func2("Returns a range of all outputs sent to this address"))
    ;
}

#endif /* blocksci_address_py_h */
