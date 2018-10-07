//
//  witness_pubkeyhash_proxy_range_map_optional.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_pubkeyhash_proxy_py.hpp"
#include "proxy/range_map_optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addWitnessPubkeyHashProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls) {
	setupRangesMapOptionalProxy(cls);
}
