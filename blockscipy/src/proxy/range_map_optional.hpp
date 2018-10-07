//
//  range_map_optional.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <blocksci/address/equiv_address.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace internal {
	constexpr auto isOptional = [](const auto &optional) { return static_cast<bool>(optional); };
	constexpr auto derefOptional = [](const auto &optional) { return *optional; };


	template <typename T>
	Iterator<typename ranges::range_value_t<T>::value_type> flattenOptional(T && t) {
	    return {ranges::view::transform(ranges::view::filter(std::forward<T>(t), isOptional), derefOptional)};
	}
}

template<typename T, typename R>
Proxy<Iterator<R>> mapOptionalProxy(Proxy<T> &p1, Proxy<ranges::optional<R>> &p2) {
	return lift(p1, [=](T && val) -> Iterator<R> {
		return internal::flattenOptional(ranges::view::transform(std::move(val), p2));
	});
}

struct AddProxyMapOptionalFuncsMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		using namespace blocksci;
		cl
		.def("map", mapOptionalProxy<T, Block>)
		.def("map", mapOptionalProxy<T, Transaction>)
		.def("map", mapOptionalProxy<T, Input>)
		.def("map", mapOptionalProxy<T, Output>)
		.def("map", mapOptionalProxy<T, AnyScript>)
		.def("map", mapOptionalProxy<T, EquivAddress>)

		.def("map", mapOptionalProxy<T, script::Pubkey>)
		.def("map", mapOptionalProxy<T, script::PubkeyHash>)
		.def("map", mapOptionalProxy<T, script::WitnessPubkeyHash>)
		.def("map", mapOptionalProxy<T, script::MultisigPubkey>)
		.def("map", mapOptionalProxy<T, script::Multisig>)
		.def("map", mapOptionalProxy<T, script::ScriptHash>)
		.def("map", mapOptionalProxy<T, script::WitnessScriptHash>)
		.def("map", mapOptionalProxy<T, script::OpReturn>)
		.def("map", mapOptionalProxy<T, script::Nonstandard>)

		.def("map", mapOptionalProxy<T, AddressType::Enum>)
		.def("map", mapOptionalProxy<T, int64_t>)
		.def("map", mapOptionalProxy<T, bool>)
		.def("map", mapOptionalProxy<T, std::chrono::system_clock::time_point>)
		.def("map", mapOptionalProxy<T, uint256>)
		.def("map", mapOptionalProxy<T, uint160>)
		.def("map", mapOptionalProxy<T, pybind11::bytes>)
		.def("map", mapOptionalProxy<T, pybind11::list>)
		.def("map", mapOptionalProxy<T, std::string>)
		;
	}
};

template <typename T>
void setupRangesMapOptionalProxy(AllProxyClasses<T> &cls) {
	cls.iterator.applyToAll(AddProxyMapOptionalFuncsMethods{});
	cls.range.applyToAll(AddProxyMapOptionalFuncsMethods{});
}


#endif /* proxy_range_map_hpp */
