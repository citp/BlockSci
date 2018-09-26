//
//  address_proxy_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "address_proxy_py.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addAddressProxyMethodsRange(AllProxyClasses<blocksci::AnyScript> &cls) {
	setupRangesProxy(cls);
}
