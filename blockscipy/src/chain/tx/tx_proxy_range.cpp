//
//  tx_proxy_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tx_proxy_py.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addTxProxyMethodsRange(AllProxyClasses<blocksci::Transaction> &cls) {
	setupRangesProxy(cls);
}
