//
//  block_proxy_optional_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "block_proxy_py.hpp"
#include "proxy/optional_range.hpp"

#include <blocksci/cluster/cluster.hpp>

void addBlockProxyMethodsOptionalRange(AllProxyClasses<blocksci::Block> &cls) {
	setupOptionalRangesProxy(cls);
}
