//
//  block_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef block_py_h
#define block_py_h

#include "blocksci_range.hpp"

#include <blocksci/chain/block.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

void init_uint256(pybind11::class_<blocksci::uint256> &cl);
void init_uint160(pybind11::class_<blocksci::uint160> &cl);
void init_block(pybind11::class_<blocksci::Block> &cl);
void addBlockRangeMethods(RangeClasses<blocksci::Block> &classes);

#endif /* block_py_h */
