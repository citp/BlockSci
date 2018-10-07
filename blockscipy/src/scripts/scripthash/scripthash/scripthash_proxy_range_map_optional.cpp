//
//  scripthash_proxy_range_map_optional.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "scripthash_proxy_py.hpp"
#include "proxy/range_map_optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addScriptHashProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::ScriptHash> &cls) {
	setupRangesMapOptionalProxy(cls);
}
