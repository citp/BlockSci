//
//  nulldata_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "nulldata_proxy_py.hpp"
#include "nulldata_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/nulldata_script.hpp>

struct AddOpReturnMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
	    func(property_tag, "data", +[](const script::OpReturn &address) {
	        return pybind11::bytes(address.getData());
	    }, "Data contained inside this address");
    }
};

void addNulldataProxyMethods(AllProxyClasses<blocksci::script::OpReturn, ProxyAddress> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddOpReturnMethods{});
    addProxyEqualityMethods(cls.base);
}
