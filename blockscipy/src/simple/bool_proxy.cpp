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
	cls.setupBasicProxy(AddProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.base.applyToAll(AddProxyBooleanMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}
