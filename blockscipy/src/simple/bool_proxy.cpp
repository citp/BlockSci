//
//  bool_proxy.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "simple_proxies.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/basic.hpp"
#include "proxy/boolean.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addBoolProxyMethods(AllProxyClasses<bool> &cls) {
	cls.applyToAll(AddProxyMethods{});
	addProxyEqualityMethods(cls.base);
	addProxyComparisonMethods(cls.base);
	addProxyBooleanMethods(cls.base);
	addProxyOptionalMethods(cls.optional);
}
