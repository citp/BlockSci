//
//  output_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "output_proxy_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>


struct AddOutputMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "address", &Output::getAddress, "This address linked to this output");
        func(property_tag, "value", &Output::getValue, "The value in base currency attached to this output");
        func(property_tag, "address_type", &Output::getType, "The address type of the output");
        func(property_tag, "is_spent", &Output::isSpent, "Returns whether this output has been spent");
        func(property_tag, "spending_tx_index", &Output::getSpendingTxIndex, "Returns the index of the tranasction which spent this output or 0 if it is unspent");
        func(property_tag, "spending_tx", &Output::getSpendingTx, "The transaction that spent this output or None if it is unspent");
        func(property_tag, "tx", &Output::transaction, "The transaction that contains this input");
        func(property_tag, "block", &Output::block, "The block that contains this input");
        func(property_tag, "index", &Output::outputIndex, "The output index inside this output's transaction");
        func(property_tag, "tx_index", &Output::txIndex, "The tx index of this output's transaction");
        ;
    }
};

struct AddOutputProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::Output>> &cl) {
		applyMethodsToProxy(cl, AddOutputMethods{});
	}
};

void addOutputProxyMethods(AllProxyClasses<blocksci::Output> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddOutputMethods{});
    addProxyEqualityMethods(cls.base);
    addProxyComparisonMethods(cls.base);
}
