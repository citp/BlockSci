//
//  scripthash_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "scripthash_proxy_py.hpp"
#include "scripts/scripthash/scripthash_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddScriptHashProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::ScriptHash>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::ScriptHash>{});
		applyMethodsToProxy(cl, AddScriptHashBaseMethods<blocksci::script::ScriptHash>{});
	}
};

void addScriptHashProxyMethods(AllProxyClasses<blocksci::script::ScriptHash> &cls) {
	addScriptHashProxyMethodsMain(cls);
	addScriptHashProxyMethodsRange(cls);
	addScriptHashProxyMethodsRangeMap(cls);
	addScriptHashProxyMethodsRangeMapOptional(cls);
	addScriptHashProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddScriptHashProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
