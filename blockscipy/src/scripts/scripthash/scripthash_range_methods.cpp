//
//  nulldata_range_methods.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "scripthash_range_py.hpp"
#include "scripthash_py.hpp"
#include "scripts/address_py.hpp"
#include "caster_py.hpp"
#include "range_apply_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/equiv_address.hpp>

using namespace blocksci;
namespace py = pybind11;

void applyMethodsToScriptHashRange(RangeClasses<script::ScriptHash> &classes) {
	applyAllMethodsToRange(classes, AddAddressMethods<script::ScriptHash>{});
    applyAllMethodsToRange(classes, AddScriptHashBaseMethods<script::ScriptHash>{});
}
