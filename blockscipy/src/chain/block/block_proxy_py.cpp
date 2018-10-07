//
//  block_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "block_proxy_py.hpp"
#include "block_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/cluster/cluster.hpp>


struct AddBlockProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::Block>> &cl) {
		applyMethodsToProxy(cl, AddBlockMethods{});
	}
};

void addBlockProxyMethods(AllProxyClasses<blocksci::Block> &cls) {
	addBlockProxyMethodsMain(cls);
	addBlockProxyMethodsRange(cls);
	addBlockProxyMethodsRangeMap(cls);
	addBlockProxyMethodsRangeMapOptional(cls);
	addBlockProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddBlockProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
