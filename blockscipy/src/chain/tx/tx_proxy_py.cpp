//
//  tx_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tx_proxy_py.hpp"
#include "tx_properties_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

struct AddProxyTransactionMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;

        func(property_tag, "ins", &Transaction::inputs, "A list of the inputs of the transaction"); // same as below
        func(property_tag, "inputs", &Transaction::inputs, "A list of the inputs of the transaction"); // same as above
        func(property_tag, "outs", &Transaction::outputs, "A list of the outputs of the transaction"); // same as below
        func(property_tag, "outputs", &Transaction::outputs, "A list of the outputs of the transaction"); // same as above
    }
};

void addTxProxyMethods(AllProxyClasses<blocksci::Transaction> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddTransactionMethods{});
    applyMethodsToProxy(cls.base, AddProxyTransactionMethods{});
    addProxyEqualityMethods(cls.base);
    addProxyComparisonMethods(cls.base);
}
