//
//  input_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef input_py_hpp
#define input_py_hpp

#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>

#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>

template <typename Class, typename FuncApplication>
void addInputMethods(Class &cl, FuncApplication func) {
	using namespace blocksci;
    cl
    .def_property_readonly("address", func([](const Input &input) {
        return input.getAddress();
    }), "This address linked to this input")
    .def_property_readonly("value", func([](const Input &input) {
        return input.getValue();
    }), "The value in base currency attached to this inout")
    .def_property_readonly("address_type", func([](const Input &input) {
        return input.getType();
    }), "The address type of the input")
    .def_property_readonly("sequence_num", func([](const Input &input) {
        return input.sequenceNumber();
    }), "The sequence number of the input")
    .def_property_readonly("spent_tx_index", func([](const Input &input) {
        return input.spentTxIndex();
    }), "The index of the transaction that this input spent")
    .def_property_readonly("spent_tx", func([](const Input &input) {
        return input.getSpentTx();
    }), "The transaction that this input spent")
    .def_property_readonly("age", func([](const Input &input) {
        return input.age();
    }), "The number of blocks between the spent output and this input")
    .def_property_readonly("tx", func([](const Input &input) {
        return input.transaction();
    }), "The transaction that contains this input")
    .def_property_readonly("block", func([](const Input &input) {
        return input.block();
    }), "The block that contains this input")
    ;
}

template <typename Class, typename FuncApplication>
void addInputRangeMethods(Class &cl, FuncApplication func) {
	using namespace blocksci;
    using Range = typename Class::type;
    cl
    .def("sent_before",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedBeforeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedAfterHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedWithinRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedOutsideRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("with_type", [=](Range &range, AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
        	return inputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    .def("with_type", [=](Range &range, ScriptType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
        	return inputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    ;
}

void init_input(pybind11::module &m);

#endif /* input_py_hpp */
