//
//  block_proxy_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "block_proxy_py.hpp"
#include "proxy/range.hpp"

#include <blocksci/cluster/cluster.hpp>

void addBlockProxyMethodsRange(AllProxyClasses<blocksci::Block> &cls) {
	setupRangesProxy(cls);
}
