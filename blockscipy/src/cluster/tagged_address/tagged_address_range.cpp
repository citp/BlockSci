//
//  cluster_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tagged_address_range_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

void addTaggedAddressRangeMethods(RangeClasses<TaggedAddress> &classes) {
    addAllRangeMethods(classes);
}