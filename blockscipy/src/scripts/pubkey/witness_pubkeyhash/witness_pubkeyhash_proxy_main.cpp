//
//  witness_pubkeyhash_proxy_main.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_pubkeyhash_proxy_py.hpp"
#include "proxy_py.hpp"
#include "proxy/basic.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addWitnessPubkeyHashProxyMethodsMain(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls) {
	cls.setupBasicProxy(AddProxyMethods{});
}
