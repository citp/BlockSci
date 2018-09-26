//
//  scripthash_optional_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "scripthash_range_py.hpp"
#include "optional_ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;
using namespace blocksci;

void addScripthashOptionalRangeMethods(RangeClasses<blocksci::script::ScriptHash> &classes) {
    addAllOptionalRangeMethods(classes);
}