//
//  address_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "address_proxy_py.hpp"
#include "address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

struct AddAddressProxyMethods {
	void operator()(pybind11::class_<Proxy<blocksci::AnyScript>> &cl) {
		applyMethodsToProxy(cl, AddAddressMethods<blocksci::AnyScript>{});
	}
};

template <blocksci::AddressType::Enum type>
using OptionalScript = ranges::optional<blocksci::ScriptAddress<type>>;

template <blocksci::AddressType::Enum type>
using OptionalScriptProxy = Proxy<OptionalScript<type>>;
using OptionalScriptProxyVariant = blocksci::to_variadic_t<blocksci::to_address_tuple_t<OptionalScriptProxy>, mpark::variant>;

template<blocksci::AddressType::Enum type>
struct ProxyScriptWithTypeFunctor {
    static OptionalScriptProxyVariant f(Proxy<blocksci::AnyScript> &p) {
    	return lift(p, [=](blocksci::AnyScript && address) -> OptionalScript<type> {
    		auto script = mpark::get_if<blocksci::ScriptAddress<type>>(&address.wrapped);
	    	if (script) {
	    		return OptionalScript<type>{*script};
	    	} else {
	    		return ranges::nullopt;
	    	}
    	});
    	
    }
};


void addAddressProxyMethods(AllProxyClasses<blocksci::AnyScript> &cls) {
	cls.base.proxy.def("with_type",  [](Proxy<blocksci::AnyScript> &p , blocksci::AddressType::Enum type) {
		static auto table = blocksci::make_dynamic_table<blocksci::AddressType, ProxyScriptWithTypeFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(p);
	});

	addAddressProxyMethodsMain(cls);
	addAddressProxyMethodsRange(cls);
	addAddressProxyMethodsRangeMap(cls);
	addAddressProxyMethodsRangeMapOptional(cls);
	addAddressProxyMethodsRangeMapSequence(cls);
	cls.optional.applyToAll(AddProxyOptionalMethods{});
	cls.optional.applyToAll(AddProxyOptionalMapMethods{});

	cls.base.applyToAll(AddAddressProxyMethods{});
	cls.base.applyToAll(AddProxyEqualityMethods{});
}
