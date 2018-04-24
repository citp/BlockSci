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

template <typename Class, typename FuncApplication, typename FuncDoc>
void addOutputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("address", func([](const Output &output) -> AnyScript {
        return output.getAddress().getScript();
    }), func2("This address linked to this output"))
    .def_property_readonly("value", func([](const Output &output) -> int64_t {
        return output.getValue();
    }), func2("The value in base currency attached to this output"))
    .def_property_readonly("address_type", func([](const Output &output) -> AddressType::Enum {
        return output.getType();
    }), func2("The address type of the output"))
    .def_property_readonly("is_spent", func([](const Output &output) -> bool {
        return output.isSpent();
    }), func2("Returns whether this output has been spent"))
    .def_property_readonly("spending_tx_index", func([](const Output &output) -> ranges::optional<int64_t> {
        return output.getSpendingTxIndex();
    }), func2("Returns the index of the tranasction which spent this output or 0 if it is unspent"))
    .def_property_readonly("spending_tx", func([](const Output &output) -> ranges::optional<Transaction> {
        return output.getSpendingTx();
    }), func2("Returns the transaction that spent this output or None if it is unspent"))
    .def_property_readonly("tx", func([](const Output &output) -> Transaction {
        return output.transaction();
    }), func2("The transaction that contains this input"))
    .def_property_readonly("block", func([](const Output &output) -> Block {
        return output.block();
    }), func2("The block that contains this input"))
    .def_property_readonly("index", func([](const Output &output) -> int64_t {
        return output.outputIndex();
    }), func2("The output index inside this output's transaction"))
    .def_property_readonly("tx_index", func([](const Output &output) -> int64_t {
        return output.txIndex();
    }), func2("The tx index of this output's transaction"))
    ;
}

#endif /* output_py_h */
