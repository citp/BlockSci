//
//  nonstandard_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "nonstandard_proxy_py.hpp"
#include "nonstandard_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/nonstandard_script.hpp>

struct AddNonstandardMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
        func(property_tag, "in_script", &blocksci::script::Nonstandard::inputString, "Nonstandard input script");
        func(property_tag, "out_script", &blocksci::script::Nonstandard::outputString, "Nonstandard output script");
    }
};

void addNonstandardProxyMethods(AllProxyClasses<blocksci::script::Nonstandard> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddNonstandardMethods{});
    addProxyEqualityMethods(cls.base);
}
