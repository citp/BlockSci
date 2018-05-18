//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_equiv_address_py_h
#define blocksci_equiv_address_py_h

#include "method_tags.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <pybind11/pybind11.h>

void init_equiv_address(pybind11::class_<blocksci::EquivAddress> &cl);

struct AddEquivAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        namespace py = pybind11;
        func(property_tag, "addresses", +[](EquivAddress &address) -> ranges::any_view<blocksci::Address> {
            return {address};
        }, "Returns an iterator over the addresses that make up this equivalent address");
        func(property_tag, "is_script_equiv", &EquivAddress::isScriptEquiv, "Returns whether this equiv address is script equivalent or not");
        func(method_tag, "balance", &EquivAddress::calculateBalance, "Calculates the balance held by these equivalent addresses at the height (Defaults to the full chain)", py::arg("height") = -1);
        func(method_tag, "outs", &EquivAddress::getOutputs, "Returns a list of all outputs sent to these equivalent addresses");
        func(method_tag, "out_txes_count", +[](EquivAddress &address) -> int64_t {
            return address.getOutputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an output");
        func(method_tag, "in_txes_count", +[](EquivAddress &address) -> int64_t {
            return address.getInputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an input");
    }
};

#endif /* blocksci_equiv_address_py_h */
