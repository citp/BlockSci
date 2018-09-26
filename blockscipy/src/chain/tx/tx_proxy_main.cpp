//
//  tx_proxy_main.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tx_proxy_py.hpp"
#include "proxy_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/block.hpp>

void addTxProxyMethodsMain(AllProxyClasses<blocksci::Transaction> &cls) {
	cls.setupBasicProxy();
}
