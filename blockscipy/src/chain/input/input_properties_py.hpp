//
//  input_properties_py.hpp
//  blockscipy
//
//  Created by Malte Moeser on 8/26/19.
//

#ifndef input_properties_py_h
#define input_properties_py_h

#include "method_tags.hpp"

#include <blocksci/chain/block.hpp>

struct AddInputMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "value", &Input::getValue, "The value in base currency attached to this input");
        func(property_tag, "address_type", &Input::getType, "The address type of the input");
        func(property_tag, "sequence_num", &Input::sequenceNumber, "The sequence number of the input");
        func(property_tag, "spent_tx_index", &Input::spentTxIndex, "The index of the transaction that this input spent");
        func(property_tag, "spent_tx", &Input::getSpentTx, "The transaction that this input spent");
        func(property_tag, "spent_output", &Input::getSpentOutput, "The output that this input spent");
        func(property_tag, "age", &Input::age, "The number of blocks between the spent output and this input");
        func(property_tag, "tx", &Input::transaction, "The transaction that contains this input");
        func(property_tag, "block", &Input::block, "The block that contains this input");
        func(property_tag, "index", &Input::inputIndex, "The index inside this transaction's inputs");
        func(property_tag, "tx_index", &Input::txIndex, "The tx index of this input's transaction");
        ;
    }
};

#endif /* input_properties_py_h */
