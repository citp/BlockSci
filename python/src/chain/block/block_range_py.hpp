//
//  block_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef block_range_py_h
#define block_range_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

void addBlockRangeMethods(RangeClasses<blocksci::Block> &classes);
void applyMethodsToBlockRange(RangeClasses<blocksci::Block> &classes);

#endif /* block_py_h */
