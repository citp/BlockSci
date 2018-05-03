//
//  tx_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_range_py_h
#define tx_range_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/transaction.hpp>

void addTxRangeMethods(RangeClasses<blocksci::Transaction> &classes);
void applyMethodsToTxRange(RangeClasses<blocksci::Transaction> &classes);
void applyRangeFiltersToTxRange(RangeClasses<blocksci::Transaction> &classes);

#endif /* tx_range_py_h */
