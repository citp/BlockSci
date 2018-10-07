//
//  multisig_pubkey_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "multisig_pubkey_proxy_py.hpp"
#include "scripts/pubkey/pubkey_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddMultisigPubkeyProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::script::MultisigPubkey>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::script::MultisigPubkey>{});
		applyMethodsToProxy(cl, AddPubkeyBaseMethods<blocksci::script::MultisigPubkey>{});
	}
};

void addMultisigPubkeyProxyMethods(AllProxyClasses<blocksci::script::MultisigPubkey> &cls) {
	addMultisigPubkeyProxyMethodsMain(cls);
	addMultisigPubkeyProxyMethodsRange(cls);
	addMultisigPubkeyProxyMethodsRangeMap(cls);
	addMultisigPubkeyProxyMethodsRangeMapOptional(cls);
	addMultisigPubkeyProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddMultisigPubkeyProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
