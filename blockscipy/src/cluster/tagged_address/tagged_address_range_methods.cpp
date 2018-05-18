//
//  tagged_address_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tagged_address_range_py.hpp"
#include "tagged_address_py.hpp"
#include "range_apply_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToTaggedAddressRange(RangeClasses<TaggedAddress> &classes) {
    applyAllMethodsToRange(classes, AddTaggedAddressMethods{});
}
