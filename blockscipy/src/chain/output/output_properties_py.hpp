//
//  output_properties_py.hpp
//  blockscipy
//
//  Created by Malte Moeser on 8/26/19.
//

#ifndef output_properties_py_h
#define output_properties_py_h

#include "method_tags.hpp"

#include <blocksci/chain/block.hpp>


struct AddOutputMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "value", &Output::getValue, "The value in base currency attached to this output");
        func(property_tag, "address_type", &Output::getType, "The address type of the output");
        func(property_tag, "is_spent", &Output::isSpent, "Returns whether this output has been spent");
        func(property_tag, "spending_tx_index", &Output::getSpendingTxIndex, "Returns the index of the tranasction which spent this output or 0 if it is unspent");
        func(property_tag, "spending_tx", &Output::getSpendingTx, "The transaction that spent this output or None if it is unspent");
        func(property_tag, "spending_input", &Output::getSpendingInput, "The input that spent this output or None if it is unspent");
        func(property_tag, "tx", &Output::transaction, "The transaction that contains this input");
        func(property_tag, "block", &Output::block, "The block that contains this input");
        func(property_tag, "index", &Output::outputIndex, "The output index inside this output's transaction");
        func(property_tag, "tx_index", &Output::txIndex, "The tx index of this output's transaction");
        ;
    }
};


#endif /* output_properties_py_h */
