//
//  tx_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_py_h
#define tx_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

#include <pybind11/pybind11.h>

void init_tx(pybind11::class_<blocksci::Transaction> &cl);
void addTxRangeMethods(RangeClasses<blocksci::Transaction> &classes);

#endif /* tx_py_h */
