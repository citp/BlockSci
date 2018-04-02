//
//  input_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef input_py_hpp
#define input_py_hpp

#include "variant_py.hpp"

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>

void init_input(pybind11::module &m);

template <typename Class, typename FuncApplication, typename FuncDoc>
void addInputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address", func([](const Input &input) {
        return input.getAddress().getScript().wrapped;
    }), func2("The address linked to this input"))
    .def_property_readonly("value", func([](const Input &input) {
        return input.getValue();
    }), func2("The value in base currency attached to this input"))
    .def_property_readonly("address_type", func([](const Input &input) {
        return input.getType();
    }), func2("The address type of the input"))
    .def_property_readonly("sequence_num", func([](const Input &input) {
        return input.sequenceNumber();
    }), func2("The sequence number of the input"))
    .def_property_readonly("spent_tx_index", func([](const Input &input) {
        return input.spentTxIndex();
    }), func2("The index of the transaction that this input spent"))
    .def_property_readonly("spent_tx", func([](const Input &input) {
        return input.getSpentTx();
    }), func2("The transaction that this input spent"))
    .def_property_readonly("age", func([](const Input &input) {
        return input.age();
    }), func2("The number of blocks between the spent output and this input"))
    .def_property_readonly("tx", func([](const Input &input) {
        return input.transaction();
    }), func2("The transaction that contains this input"))
    .def_property_readonly("block", func([](const Input &input) {
        return input.block();
    }), func2("The block that contains this input"))
    .def_property_readonly("index", func([](const Input &input) {
        return input.inputIndex();
    }), func2("The index index inside this input's transaction"))
    .def_property_readonly("tx_index", func([](const Input &input) {
        return input.txIndex();
    }), func2("The tx index of this input's transaction"))
    ;
}

template <typename Class, typename FuncApplication>
void addInputRangeMethods(Class &cl, FuncApplication func) {
    using namespace blocksci;
    using Range = typename Class::type;
    cl
    .def("sent_before",  [=](Range &range, BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<Input> {
            return inputsCreatedBeforeHeight(r, height);
        });
    }, "Returns a range including the subset of inputs which spent an output created before the given height")
    .def("sent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedAfterHeight(r, height);
        });
    }, "Returns a range including the subset of inputs which spent an output created after the given height")
    .def("sent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedWithinRelativeHeight(r, height);
        });
    }, "Returns a range including the subset of inputs which spent an output created more than a given number of blocks before the input")
    .def("sent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedOutsideRelativeHeight(r, height);
        });
    }, "Returns a range including the subset of inputs which spent an output created less than a given number of blocks before the input")
    .def("with_type", [=](Range &range, AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
        	return inputsOfType(r, type);
        });
    }, "Return a range including only inputs sent to the given address type")
    ;
}

#endif /* input_py_hpp */
