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

blocksci::AnyScript pyInputAddress(const blocksci::Input &input) {
    return input.getAddress().getScript();
}

int64_t pyInputValue(const blocksci::Input &input) {
    return input.getValue();
}

blocksci::AddressType::Enum pyInputAddressType(const blocksci::Input &input) {
    return input.getType();
}

int64_t pyInputSequenceNum(const blocksci::Input &input) {
    return input.sequenceNumber();
}

int64_t pyInputSpentTxIndex(const blocksci::Input &input) {
    return input.spentTxIndex();
}

blocksci::Transaction pyInputSpentTx(const blocksci::Input &input) {
    return input.getSpentTx();
}

int64_t pyInputAge(const blocksci::Input &input) {
    return input.age();
}

blocksci::Transaction pyInputTx(const blocksci::Input &input) {
    return input.transaction();
}

blocksci::Block pyInputBlock(const blocksci::Input &input) {
    return input.block();
}

int64_t pyInputIndex(const blocksci::Input &input) {
    return input.inputIndex();
}

int64_t pyInputTxIndex(const blocksci::Input &input) {
    return input.txIndex();
}


template <typename Class, typename FuncApplication, typename FuncDoc>
void addInputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address", func(pyInputAddress), func2("The address linked to this input"))
    .def_property_readonly("value", func(pyInputValue), func2("The value in base currency attached to this input"))
    .def_property_readonly("address_type", func(pyInputAddressType), func2("The address type of the input"))
    .def_property_readonly("sequence_num", func(pyInputSequenceNum), func2("The sequence number of the input"))
    .def_property_readonly("spent_tx_index", func(pyInputSpentTxIndex), func2("The index of the transaction that this input spent"))
    .def_property_readonly("spent_tx", func(pyInputSpentTx), func2("The transaction that this input spent"))
    .def_property_readonly("age", func(pyInputAge), func2("The number of blocks between the spent output and this input"))
    .def_property_readonly("tx", func(pyInputTx), func2("The transaction that contains this input"))
    .def_property_readonly("block", func(pyInputBlock), func2("The block that contains this input"))
    .def_property_readonly("index", func(pyInputIndex), func2("The index index inside this input's transaction"))
    .def_property_readonly("tx_index", func(pyInputTxIndex), func2("The tx index of this input's transaction"))
    ;
}

#endif /* input_py_hpp */
