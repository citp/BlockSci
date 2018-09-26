//
//  address_proxy_optional_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "address_proxy_py.hpp"
#include "proxy/optional_range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addAddressProxyMethodsOptionalRange(AllProxyClasses<blocksci::AnyScript> &cls) {
	setupOptionalRangesProxy(cls);
}
