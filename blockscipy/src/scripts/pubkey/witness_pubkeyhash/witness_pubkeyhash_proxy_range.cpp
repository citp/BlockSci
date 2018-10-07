//
//  witness_pubkeyhash_proxy_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_pubkeyhash_proxy_py.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addWitnessPubkeyHashProxyMethodsRange(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls) {
	setupRangesProxy(cls);
}
