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
        func(property_tag, "address", &Input::getAddress, "The address linked to this input");
        func(property_tag, "value", &Input::getValue, "The value in base currency attached to this input");
        func(property_tag, "address_type", &Input::getType, "The address type of the input");
        func(property_tag, "sequence_num", &Input::sequenceNumber, "The sequence number of the input");
        func(property_tag, "spent_tx_index", &Input::spentTxIndex, "The index of the transaction that this input spent");
        func(property_tag, "spent_tx", &Input::getSpentTx, "The transaction that this input spent");
        func(property_tag, "age", &Input::age, "The number of blocks between the spent output and this input");
        func(property_tag, "tx", &Input::transaction, "The transaction that contains this input");
        func(property_tag, "block", &Input::block, "The block that contains this input");
        func(property_tag, "index", &Input::inputIndex, "The index index inside this input's transaction");
        func(property_tag, "tx_index", &Input::txIndex, "The tx index of this input's transaction");
        ;
    }
};

#endif /* input_py_hpp */
