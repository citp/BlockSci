//
//  output_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef output_py_h
#define output_py_h

#include "python_fwd.hpp"
#include "method_tags.hpp"

#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>

void init_output(pybind11::class_<blocksci::Output> &cl);
void addOutputRangeMethods(RangeClasses<blocksci::Output> &classes);

struct AddOutputMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "address", &Output::getAddress, "This address linked to this output");
        func(property_tag, "value", &Output::getValue, "The value in base currency attached to this output");
        func(property_tag, "address_type", &Output::getType, "The address type of the output");
        func(property_tag, "is_spent", &Output::isSpent, "Returns whether this output has been spent");
        func(property_tag, "spending_tx_index", &Output::getSpendingTxIndex, "Returns the index of the tranasction which spent this output or 0 if it is unspent");
        func(property_tag, "spending_tx", &Output::getSpendingTx, "The transaction that spent this output or None if it is unspent");
        func(property_tag, "tx", &Output::transaction, "The transaction that contains this input");
        func(property_tag, "block", &Output::block, "The block that contains this input");
        func(property_tag, "index", &Output::outputIndex, "The output index inside this output's transaction");
        func(property_tag, "tx_index", &Output::txIndex, "The tx index of this output's transaction");
        ;
    }
};

#endif /* output_py_h */
