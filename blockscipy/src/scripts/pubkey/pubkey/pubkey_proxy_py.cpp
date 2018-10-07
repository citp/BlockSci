//
//  pubkey_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "pubkey_proxy_py.hpp"
#include "scripts/pubkey/pubkey_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddPubkeyProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::Pubkey>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::Pubkey>{});
		applyMethodsToProxy(cl, AddPubkeyBaseMethods<blocksci::script::Pubkey>{});
	}
};

void addPubkeyProxyMethods(AllProxyClasses<blocksci::script::Pubkey> &cls) {
	addPubkeyProxyMethodsMain(cls);
	addPubkeyProxyMethodsRange(cls);
	addPubkeyProxyMethodsRangeMap(cls);
	addPubkeyProxyMethodsRangeMapOptional(cls);
	addPubkeyProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddPubkeyProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
