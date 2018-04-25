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
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

void init_output(pybind11::module &m);

blocksci::AnyScript pyOutputAddress(const blocksci::Output &output) {
    return output.getAddress().getScript();
}

int64_t pyOutputValue(const blocksci::Output &output) {
    return output.getValue();
}

blocksci::AddressType::Enum pyOutputAddressType(const blocksci::Output &output) {
    return output.getType();
}

bool pyOutputIsSpent(const blocksci::Output &output) {
    return output.isSpent();
}

ranges::optional<int64_t> pyOutputSpendingTxIndex(const blocksci::Output &output) {
    return output.getSpendingTxIndex();
}

ranges::optional<blocksci::Transaction> pyOutputSpendingTx(const blocksci::Output &output) {
    return output.getSpendingTx();
}

blocksci::Transaction pyOutputTx(const blocksci::Output &output) {
    return output.transaction();
}

blocksci::Block pyOutputBlock(const blocksci::Output &output) {
    return output.block();
}

int64_t pyOutputIndex(const blocksci::Output &output) {
    return output.outputIndex();
}

int64_t pyOutputTxIndex(const blocksci::Output &output) {
    return output.txIndex();
}

template <typename Class, typename FuncApplication, typename FuncDoc>
void addOutputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("address", func(pyOutputAddress), func2("This address linked to this output"))
    .def_property_readonly("value", func(pyOutputValue), func2("The value in base currency attached to this output"))
    .def_property_readonly("address_type", func(pyOutputAddressType), func2("The address type of the output"))
    .def_property_readonly("is_spent", func(pyOutputIsSpent), func2("Returns whether this output has been spent"))
    .def_property_readonly("spending_tx_index", func(pyOutputSpendingTxIndex), func2("Returns the index of the tranasction which spent this output or 0 if it is unspent"))
    .def_property_readonly("spending_tx", func(pyOutputSpendingTx), func2("Returns the transaction that spent this output or None if it is unspent"))
    .def_property_readonly("tx", func(pyOutputTx), func2("The transaction that contains this input"))
    .def_property_readonly("block", func(pyOutputBlock), func2("The block that contains this input"))
    .def_property_readonly("index", func(pyOutputIndex), func2("The output index inside this output's transaction"))
    .def_property_readonly("tx_index", func(pyOutputTxIndex), func2("The tx index of this output's transaction"))
    ;
}

#endif /* output_py_h */
