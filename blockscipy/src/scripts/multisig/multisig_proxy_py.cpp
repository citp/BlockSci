//
//  multisig_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "multisig_proxy_py.hpp"
#include "multisig_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddMultisigProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::Multisig>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::Multisig>{});
		applyMethodsToProxy(cl, AddMultisigMethods{});
	}
};

void addMultisigProxyMethods(AllProxyClasses<blocksci::script::Multisig> &cls) {
	addMultisigProxyMethodsMain(cls);
	addMultisigProxyMethodsRange(cls);
	addMultisigProxyMethodsRangeMap(cls);
	addMultisigProxyMethodsRangeMapOptional(cls);
	addMultisigProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddMultisigProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
