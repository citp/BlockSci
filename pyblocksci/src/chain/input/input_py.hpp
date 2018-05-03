//
//  input_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef input_py_hpp
#define input_py_hpp

#include "method_tags.hpp"

#include <blocksci/chain/input.hpp>

#include <pybind11/pybind11.h>


void init_input(pybind11::class_<blocksci::Input> &cl);

struct AddInputMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        namespace py = pybind11;
        using namespace blocksci;
        func(property_tag, "address", &blocksci::Input::getAddress, "The address linked to this input");
        func(property_tag, "value", &blocksci::Input::getValue, "The value in base currency attached to this input");
        func(property_tag, "address_type", &blocksci::Input::getType, "The address type of the input");
        func(property_tag, "sequence_num", &blocksci::Input::sequenceNumber, "The sequence number of the input");
        func(property_tag, "spent_tx_index", &blocksci::Input::spentTxIndex, "The index of the transaction that this input spent");
        func(property_tag, "spent_tx", &blocksci::Input::getSpentTx, "The transaction that this input spent");
        func(property_tag, "age", &blocksci::Input::age, "The number of blocks between the spent output and this input");
        func(property_tag, "tx", &blocksci::Input::transaction, "The transaction that contains this input");
        func(property_tag, "block", &blocksci::Input::block, "The block that contains this input");
        func(property_tag, "index", &blocksci::Input::inputIndex, "The index index inside this input's transaction");
        func(property_tag, "tx_index", &blocksci::Input::txIndex, "The tx index of this input's transaction");
        ;
    }
};

#endif /* input_py_hpp */
