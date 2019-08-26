//
//  input_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "input_proxy_py.hpp"
#include "input_properties_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/comparison.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"


struct AddInputProxyMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;

        func(property_tag, "address", &Input::getAddress, "The address linked to this input");
        ;
    }
};


void addInputProxyMethods(AllProxyClasses<blocksci::Input> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddInputMethods{});
    applyMethodsToProxy(cls.base, AddInputProxyMethods{});
    addProxyEqualityMethods(cls.base);
    addProxyComparisonMethods(cls.base);
}
