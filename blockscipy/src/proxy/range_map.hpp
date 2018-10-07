//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <blocksci/address/equiv_address.hpp>

#include <range/v3/view/transform.hpp>

template<typename T, ranges::category range_cat, typename R>
Proxy<ranges::any_view<R, range_cat>> mapSimpleProxy(Proxy<ranges::any_view<T, range_cat>> &p1, Proxy<R> &p2) {
	return lift(p1, [=](ranges::any_view<T, range_cat> && val) -> ranges::any_view<R, range_cat> {
		return ranges::view::transform(std::move(val), p2);
	});
}

struct AddProxyMapFuncsMethods {
	template<typename T, ranges::category range_cat>
	void operator()(pybind11::class_<Proxy<ranges::any_view<T, range_cat>>> &cl) {
		using namespace blocksci;
		cl
		.def("map", mapSimpleProxy<T, range_cat, Block>)
		.def("map", mapSimpleProxy<T, range_cat, Transaction>)
		.def("map", mapSimpleProxy<T, range_cat, Input>)
		.def("map", mapSimpleProxy<T, range_cat, Output>)
		.def("map", mapSimpleProxy<T, range_cat, AnyScript>)
		.def("map", mapSimpleProxy<T, range_cat, EquivAddress>)

		.def("map", mapSimpleProxy<T, range_cat, script::Pubkey>)
		.def("map", mapSimpleProxy<T, range_cat, script::PubkeyHash>)
		.def("map", mapSimpleProxy<T, range_cat, script::WitnessPubkeyHash>)
		.def("map", mapSimpleProxy<T, range_cat, script::MultisigPubkey>)
		.def("map", mapSimpleProxy<T, range_cat, script::Multisig>)
		.def("map", mapSimpleProxy<T, range_cat, script::ScriptHash>)
		.def("map", mapSimpleProxy<T, range_cat, script::WitnessScriptHash>)
		.def("map", mapSimpleProxy<T, range_cat, script::OpReturn>)
		.def("map", mapSimpleProxy<T, range_cat, script::Nonstandard>)

		.def("map", mapSimpleProxy<T, range_cat, AddressType::Enum>)
		.def("map", mapSimpleProxy<T, range_cat, int64_t>)
		.def("map", mapSimpleProxy<T, range_cat, bool>)
		.def("map", mapSimpleProxy<T, range_cat, std::chrono::system_clock::time_point>)
		.def("map", mapSimpleProxy<T, range_cat, uint256>)
		.def("map", mapSimpleProxy<T, range_cat, uint160>)
		.def("map", mapSimpleProxy<T, range_cat, pybind11::bytes>)
		.def("map", mapSimpleProxy<T, range_cat, pybind11::list>)
		.def("map", mapSimpleProxy<T, range_cat, std::string>)
		;
	}
};

template <typename T>
void setupRangesMapProxy(AllProxyClasses<T> &cls) {
	cls.iterator.applyToAll(AddProxyMapFuncsMethods{});
	cls.range.applyToAll(AddProxyMapFuncsMethods{});
}


#endif /* proxy_range_map_hpp */
