//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_address_py_h
#define blocksci_address_py_h

#include "method_tags.hpp"
#include "func_converter.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <pybind11/pybind11.h>

void init_address_type(pybind11::module &m);
void init_address(pybind11::class_<blocksci::ScriptBase> &addressCl);
void init_equiv_address(pybind11::class_<blocksci::EquivAddress> &cl);

template <typename T>
struct AddAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "address_num", toFunc(&T::getScriptNum), "The internal identifier of the address");
        func(property_tag, "type", toFunc(&T::getType), "The type of address");
        func(property_tag, "raw_type", toFunc([](T &address) {
            return static_cast<int64_t>(address.getType());
        }), "The type of address");
        func(method_tag, "equiv", toFunc(&T::getEquivAddresses), "Returns a list of all addresses equivalent to this address", pybind11::arg("equiv_script") = true);
        func(method_tag, "balance", toFunc(&T::calculateBalance), "Calculates the balance held by this address at the height (Defaults to the full chain)", pybind11::arg("height") = -1);
        func(method_tag, "out_txes_count", toFunc([](T &address) {
            return ranges::distance(address.getOutputTransactions());
        }), "Return the number of transactions where this address was an output");
        func(method_tag, "in_txes_count", toFunc([](T &address) {
            return ranges::distance(address.getInputTransactions());
        }), "Return the number of transactions where this address was an input");
        func(property_tag, "first_tx", toFunc(&T::getFirstTransaction), "Get the first transaction that was sent to a type equivalent address");
        func(property_tag, "revealed_tx", toFunc(&T::getTransactionRevealed), "The transaction where a type equivalent address was first revealed");
        func(property_tag, "has_been_spent", toFunc(&T::hasBeenSpent), "Check if a type equivalent address has ever been spent");
        func(property_tag, "outs", toFunc(&T::getOutputs), "Returns a range of all outputs sent to this address");
        ;
    }
};

#endif /* blocksci_address_py_h */
