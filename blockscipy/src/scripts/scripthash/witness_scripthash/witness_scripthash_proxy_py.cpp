//
//  witness_scripthash_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_scripthash_proxy_py.hpp"
#include "scripts/scripthash/scripthash_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddWitnessScriptHashProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::WitnessScriptHash>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::WitnessScriptHash>{});
		applyMethodsToProxy(cl, AddScriptHashBaseMethods<blocksci::script::WitnessScriptHash>{});
	}
};

void addWitnessScriptHashProxyMethods(AllProxyClasses<blocksci::script::WitnessScriptHash> &cls) {
	addWitnessScriptHashProxyMethodsMain(cls);
	addWitnessScriptHashProxyMethodsRange(cls);
	addWitnessScriptHashProxyMethodsRangeMap(cls);
	addWitnessScriptHashProxyMethodsRangeMapOptional(cls);
	addWitnessScriptHashProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddWitnessScriptHashProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
