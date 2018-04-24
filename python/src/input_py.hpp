//
//  input_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef input_py_hpp
#define input_py_hpp

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>

void init_input(pybind11::module &m);

template <typename Class, typename FuncApplication, typename FuncDoc>
void addInputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address", func([](const Input &input) -> AnyScript {
        return input.getAddress().getScript();
    }), func2("The address linked to this input"))
    .def_property_readonly("value", func([](const Input &input) -> int64_t {
        return input.getValue();
    }), func2("The value in base currency attached to this input"))
    .def_property_readonly("address_type", func([](const Input &input) -> AddressType::Enum {
        return input.getType();
    }), func2("The address type of the input"))
    .def_property_readonly("sequence_num", func([](const Input &input) -> int64_t {
        return input.sequenceNumber();
    }), func2("The sequence number of the input"))
    .def_property_readonly("spent_tx_index", func([](const Input &input) -> int64_t {
        return input.spentTxIndex();
    }), func2("The index of the transaction that this input spent"))
    .def_property_readonly("spent_tx", func([](const Input &input) -> Transaction {
        return input.getSpentTx();
    }), func2("The transaction that this input spent"))
    .def_property_readonly("age", func([](const Input &input) -> int64_t {
        return input.age();
    }), func2("The number of blocks between the spent output and this input"))
    .def_property_readonly("tx", func([](const Input &input) -> Transaction {
        return input.transaction();
    }), func2("The transaction that contains this input"))
    .def_property_readonly("block", func([](const Input &input) -> Block {
        return input.block();
    }), func2("The block that contains this input"))
    .def_property_readonly("index", func([](const Input &input) -> int64_t {
        return input.inputIndex();
    }), func2("The index index inside this input's transaction"))
    .def_property_readonly("tx_index", func([](const Input &input) -> int64_t {
        return input.txIndex();
    }), func2("The tx index of this input's transaction"))
    ;
}

#endif /* input_py_hpp */
