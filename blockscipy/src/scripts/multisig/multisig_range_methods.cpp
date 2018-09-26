//
//  multisig_range_methods.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "multisig_range_py.hpp"
#include "multisig_py.hpp"
#include "scripts/address_py.hpp"
#include "caster_py.hpp"
#include "range_apply_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/block.hpp>

using namespace blocksci;
namespace py = pybind11;

void applyMethodsToMultisigRange(RangeClasses<script::Multisig> &classes) {
	applyAllMethodsToRange(classes, AddAddressMethods<script::Multisig>{});
    applyAllMethodsToRange(classes, AddMultisigMethods{});
}
