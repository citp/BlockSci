//
//  address_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "address_proxy_py.hpp"
#include "address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddAddressProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, blocksci::AnyScript>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::AnyScript>{});
	}
};

void addAddressProxyMethods(AllProxyClasses<blocksci::AnyScript> &cls) {
	addAddressProxyMethodsMain(cls);
	setupRangesProxy(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddAddressProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
