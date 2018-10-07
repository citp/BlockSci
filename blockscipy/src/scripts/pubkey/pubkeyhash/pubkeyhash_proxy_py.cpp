//
//  pubkeyhash_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "pubkeyhash_proxy_py.hpp"
#include "scripts/pubkey/pubkey_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddPubkeyHashProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::PubkeyHash>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::PubkeyHash>{});
		applyMethodsToProxy(cl, AddPubkeyBaseMethods<blocksci::script::PubkeyHash>{});
	}
};

void addPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::PubkeyHash> &cls) {
	addPubkeyHashProxyMethodsMain(cls);
	addPubkeyHashProxyMethodsRange(cls);
	addPubkeyHashProxyMethodsRangeMap(cls);
	addPubkeyHashProxyMethodsRangeMapOptional(cls);
	addPubkeyHashProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddPubkeyHashProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
