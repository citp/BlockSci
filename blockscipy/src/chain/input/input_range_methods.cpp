//
//  input_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_py.hpp"
#include "range_apply_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToInputRange(RangeClasses<Input> &classes) {
    applyAllMethodsToRange(classes, AddInputMethods{});
}
