//
//  witness_pubkeyhash_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_pubkeyhash_proxy_py.hpp"
#include "scripts/pubkey/pubkey_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddWitnessPubkeyHashProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::WitnessPubkeyHash>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::WitnessPubkeyHash>{});
		applyMethodsToProxy(cl, AddPubkeyBaseMethods<blocksci::script::WitnessPubkeyHash>{});
	}
};

void addWitnessPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls) {
	addWitnessPubkeyHashProxyMethodsMain(cls);
	addWitnessPubkeyHashProxyMethodsRange(cls);
	addWitnessPubkeyHashProxyMethodsRangeMap(cls);
	addWitnessPubkeyHashProxyMethodsRangeMapOptional(cls);
	addWitnessPubkeyHashProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddWitnessPubkeyHashProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
