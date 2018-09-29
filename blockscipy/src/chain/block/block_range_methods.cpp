//
//  block_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "block_range_py.hpp"
#include "range_apply_py.hpp"
#include "block_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToBlockRange(RangeClasses<blocksci::Block> &classes) {
    applyAllMethodsToRange(classes, AddBlockMethods{});
}
