//
//  input_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "input_proxy_py.hpp"
#include "input_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddInputProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, blocksci::Input>> &cl) {
		applyMethodsToProxy(cl, AddInputMethods{});
	}
};

void addInputProxyMethods(AllProxyClasses<blocksci::Input> &cls) {
	addInputProxyMethodsMain(cls);
	addInputProxyMethodsRange(cls);
	addInputProxyMethodsOptionalRange(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddInputProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
}
