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
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

using namespace blocksci;

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

struct AddAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "address_num", &AnyScript::getScriptNum, "The internal identifier of the address");
        func(property_tag, "type", &AnyScript::getType, "The type of address");
        func(property_tag, "raw_type", +[](AnyScript &address) {
            return static_cast<int64_t>(address.getType());
        }, "The type of address");
        func(method_tag, "equiv", &AnyScript::getEquivAddresses, "Returns a list of all addresses equivalent to this address", pybind11::arg("equiv_script") = true);
        func(method_tag, "balance", &AnyScript::calculateBalance, "Calculates the balance held by this address at the height (Defaults to the full chain)", pybind11::arg("height") = -1);
        func(method_tag, "out_txes_count", +[](AnyScript &address) -> int64_t {
            return ranges::distance(address.getOutputTransactions());
        }, "Return the number of transactions where this address was an output");
        func(method_tag, "in_txes_count", +[](AnyScript &address) -> int64_t {
            return ranges::distance(address.getInputTransactions());
        }, "Return the number of transactions where this address was an input");
        func(property_tag, "first_tx", &AnyScript::getFirstTransaction, "Get the first transaction that was sent to a type equivalent address");
        func(property_tag, "revealed_tx", &AnyScript::getTransactionRevealed, "The transaction where a type equivalent address was first revealed");
        func(property_tag, "has_been_spent", &AnyScript::hasBeenSpent, "Check if a type equivalent address has ever been spent");
        func(property_tag, "outs", +[](AnyScript &address) -> Iterator<Output> {
            return address.getOutputs();
        }, "Returns a iterator over all outputs sent to this address");
        func(property_tag, "ins", +[](AnyScript &address) -> Iterator<Input> {
            return address.getInputs();
        }, "Returns an iterator over all inputs spent from this address");
        func(property_tag, "out_txes", +[](AnyScript &address) -> Iterator<Transaction> {
            return address.getOutputTransactions();
        }, "Returns an iterator over all transaction where this address was an output");
    }
};

void init_proxy_address(pybind11::class_<ProxyAddress> &addressCl) {
	applyMethodsToProxyGeneric(addressCl, AddAddressMethods{});
}

void addAddressProxyMethods(AllProxyClasses<blocksci::AnyScript> &cls) {
	cls.base.def("with_type",  [](Proxy<blocksci::AnyScript> &p , blocksci::AddressType::Enum type) {
		static auto table = blocksci::make_dynamic_table<blocksci::AddressType, ProxyScriptWithTypeFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(p);
	});

	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);
    addProxyOptionalMapMethods(cls.optional);

    addProxyEqualityMethods(cls.base);
}
