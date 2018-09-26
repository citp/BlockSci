//
//  nonstandard_range_methods.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nonstandard_range_py.hpp"
#include "nonstandard_py.hpp"
#include "scripts/address_py.hpp"
#include "caster_py.hpp"
#include "range_apply_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>

using namespace blocksci;
namespace py = pybind11;

void applyMethodsToNonstandardRange(RangeClasses<script::Nonstandard> &classes) {
	applyAllMethodsToRange(classes, AddAddressMethods<script::Nonstandard>{});
    applyAllMethodsToRange(classes, AddNonstandardMethods{});
}
