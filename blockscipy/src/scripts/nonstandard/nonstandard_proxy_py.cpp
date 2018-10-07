//
//  nonstandard_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "nonstandard_proxy_py.hpp"
#include "nonstandard_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddNonstandardProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::Nonstandard>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::Nonstandard>{});
		applyMethodsToProxy(cl, AddNonstandardMethods{});
	}
};

void addNonstandardProxyMethods(AllProxyClasses<blocksci::script::Nonstandard> &cls) {
	addNonstandardProxyMethodsMain(cls);
	addNonstandardProxyMethodsRange(cls);
	addNonstandardProxyMethodsRangeMap(cls);
	addNonstandardProxyMethodsRangeMapOptional(cls);
	addNonstandardProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddNonstandardProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
