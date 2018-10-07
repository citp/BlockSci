//
//  witness_scripthash_proxy_main.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "witness_scripthash_proxy_py.hpp"
#include "proxy_py.hpp"
#include "proxy/basic.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addWitnessScriptHashProxyMethodsMain(AllProxyClasses<blocksci::script::WitnessScriptHash> &cls) {
	cls.setupBasicProxy(AddProxyMethods{});
}
