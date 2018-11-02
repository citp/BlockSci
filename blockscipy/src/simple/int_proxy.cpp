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
	cls.applyToAll(AddProxyMethods{});
	addProxyEqualityMethods(cls.base);
	addProxyComparisonMethods(cls.base);
	addProxyArithMethods(cls.base);
	addProxyOptionalMethods(cls.optional);
	addProxyEqualityMethods(cls.optional);
	addProxyOptionalArithMethods(cls.optional);
	addProxyArithRangeMethods(cls.iterator);
	addProxyArithRangeMethods(cls.range);
}
