//
//  other_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "other_proxy_py.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/arithmetic.hpp"
#include "proxy/boolean.hpp"
#include "proxy/optional.hpp"
#include "proxy/optional_arithmetic.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

void addIntProxyMethods(AllProxyClasses<int64_t> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.base.applyToAll(AddProxyArithMethods{});
	cls.optional.applyToAll(AddProxyOptionalArithMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addBoolProxyMethods(AllProxyClasses<bool> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.base.applyToAll(AddProxyBooleanMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addAddressTypeProxyMethods(AllProxyClasses<blocksci::AddressType::Enum> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addTimeProxyMethods(AllProxyClasses<std::chrono::system_clock::time_point> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addUint256ProxyMethods(AllProxyClasses<blocksci::uint256> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addUint160ProxyMethods(AllProxyClasses<blocksci::uint160> &cls) {
	cls.setupSimplProxy();
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}

void addBytesProxyMethods(AllProxyClasses<pybind11::bytes> &cls) {
	cls.setupSimplProxy();
	cls.optional.applyToAll(AddProxyOptionalMethods{});
}
