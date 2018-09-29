//
//  tx_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tx_proxy_py.hpp"
#include "tx_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/range.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddTxProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, blocksci::Transaction>> &cl) {
		applyMethodsToProxy(cl, AddTransactionMethods{});
	}
};

void addTxProxyMethods(AllProxyClasses<blocksci::Transaction> &cls) {
	addTxProxyMethodsMain(cls);
	addTxProxyMethodsRange(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddTxProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
	cls.base.applyToAll(AddProxyComparisonMethods{});
}
