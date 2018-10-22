//
//  witness_unknown_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//
//

#include "witness_unknown_proxy_py.hpp"
#include "witness_unknown_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddWitnessUnknownMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
        func(property_tag, "witness_version", &blocksci::script::WitnessUnknown::witnessVersion, "Witness version of the unknown script");
    }
};

void addWitnessUnknownProxyMethods(AllProxyClasses<blocksci::script::WitnessUnknown> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);
    addProxyOptionalMapMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddWitnessUnknownMethods{});
    addProxyEqualityMethods(cls.base);
}
