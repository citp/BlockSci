//
//  output_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "output_proxy_py.hpp"
#include "output_properties_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

struct AddOutputProxyMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        
        func(property_tag, "address", &Output::getAddress, "This address linked to this output");
        ;
    }
};


void addOutputProxyMethods(AllProxyClasses<blocksci::Output> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddOutputMethods{});
    applyMethodsToProxy(cls.base, AddOutputProxyMethods{});
    addProxyEqualityMethods(cls.base);
    addProxyComparisonMethods(cls.base);
}
