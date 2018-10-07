//
//  nulldata_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "nulldata_proxy_py.hpp"
#include "nulldata_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddNulldataProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::OpReturn>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::OpReturn>{});
		applyMethodsToProxy(cl, AddOpReturnMethods{});
	}
};

void addNulldataProxyMethods(AllProxyClasses<blocksci::script::OpReturn> &cls) {
	addNulldataProxyMethodsMain(cls);
	addNulldataProxyMethodsRange(cls);
	addNulldataProxyMethodsRangeMap(cls);
	addNulldataProxyMethodsRangeMapOptional(cls);
	addNulldataProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddNulldataProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
