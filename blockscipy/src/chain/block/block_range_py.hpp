//
//  block_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef block_range_py_h
#define block_range_py_h

#include "python_fwd.hpp"
#include "method_types.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/block_range.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/view/any_view.hpp>


void addBlockRangeMethods(RangeClasses<blocksci::Block> &classes);

#endif /* block_py_h */
