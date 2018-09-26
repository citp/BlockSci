//
//  tagged_address_optional_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tagged_address_py.hpp"
#include "optional_ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

void addTaggedAddressOptionalRangeMethods(RangeClasses<blocksci::TaggedAddress> &classes) {
    addAllOptionalRangeMethods(classes);
}