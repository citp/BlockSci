//
//  address_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "address_range_py.hpp"
#include "range_apply_py.hpp"
#include "address_py.hpp"
#include "blocksci_range.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToAddressRange(RangeClasses<AnyScript> &classes) {
    applyAllMethodsToRange(classes, AddAddressMethods<AnyScript>{});
}
