//
//  int_proxy.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "simple_proxies.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/arithmetic.hpp"
#include "proxy/arithmetic_range.hpp"
#include "proxy/basic.hpp"
#include "proxy/optional.hpp"
#include "proxy/optional_arithmetic.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addIntProxyMethods(AllProxyClasses<int64_t> &cls) {
	cls.setupBasicProxy(AddProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.base.applyToAll(AddProxyArithMethods{});
	cls.optional.applyToAll(AddProxyOptionalArithMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});

	cls.iterator.applyToAll(AddProxyArithRangeMethods{});
	cls.range.applyToAll(AddProxyArithRangeMethods{});
}
