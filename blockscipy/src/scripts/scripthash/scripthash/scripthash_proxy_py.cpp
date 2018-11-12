//
//  scripthash_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "scripthash_proxy_py.hpp"
#include "scripts/scripthash/scripthash_proxy_impl.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

void addScriptHashProxyMethods(AllProxyClasses<blocksci::script::ScriptHash, ProxyAddress> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddScriptHashBaseMethods<blocksci::script::ScriptHash>{});
    addProxyEqualityMethods(cls.base);
}
