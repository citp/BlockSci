//
//  output_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef output_py_h
#define output_py_h

#include <pybind11/pybind11.h>

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

void init_output(pybind11::module &m);

template <typename Class, typename FuncApplication, typename FuncDoc>
void addOutputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    
    cl
    .def_property_readonly("address", func([](const Output &output) {
        return output.getAddress().getScript().wrapped;
    }), func2("This address linked to this output"))
    .def_property_readonly("value", func([](const Output &output) {
        return output.getValue();
    }), func2("The value in base currency attached to this output"))
    .def_property_readonly("address_type", func([](const Output &output) {
        return output.getType();
    }), func2("The address type of the output"))
    .def_property_readonly("is_spent", func([](const Output &output) {
        return output.isSpent();
    }), func2("Returns whether this output has been spent"))
    .def_property_readonly("spending_tx_index", func([](const Output &output) {
        return output.getSpendingTxIndex();
    }), func2("Returns the index of the tranasction which spent this output or 0 if it is unspent"))
    .def_property_readonly("spending_tx", func([](const Output &output) {
        return output.getSpendingTx();
    }), func2("Returns the transaction that spent this output or None if it is unspent"))
    .def_property_readonly("tx", func([](const Output &output) {
        return output.transaction();
    }), func2("The transaction that contains this input"))
    .def_property_readonly("block", func([](const Output &output) {
        return output.block();
    }), func2("The block that contains this input"))
    .def_property_readonly("index", func([](const Output &output) {
        return output.outputIndex();
    }), func2("The output index inside this output's transaction"))
    .def_property_readonly("tx_index", func([](const Output &output) {
        return output.txIndex();
    }), func2("The tx index of this output's transaction"))
    ;
}

template <typename Class, typename FuncApplication>
void addOutputRangeMethods(Class &cl, FuncApplication func) {
    using Range = typename Class::type;
    cl
    .def_property_readonly("unspent",  [=](Range &range) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsUnspent(r);
        });
    }, "Returns a range including the subset of outputs which were never spent")
    .def("spent_before",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentBeforeHeight(r, height);
        });
    }, "Returns a range including the subset of outputs which were spent before the given height")
    .def("spent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentAfterHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent after the given height")
    .def("spent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentWithinRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent within the given number of blocks")
    .def("spent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentOutsideRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent later than the given number of blocks")
    .def("with_type", [=](Range &range, blocksci::AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsOfType(std::forward<decltype(r)>(r), type);
        });
    }, "Returns a range including the subset of outputs which were sent to the given address type")
    ;
}

#endif /* output_py_h */
