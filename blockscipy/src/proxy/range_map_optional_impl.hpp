//
//  range_map_optional_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_optional_impl_hpp
#define proxy_range_map_optional_impl_hpp

#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

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

template<ranges::category range_cat, typename R>
Proxy<Iterator<R>> mapOptional(ProxySequence<range_cat> &seq, Proxy<ranges::optional<R>> &p2) {
	auto generic = seq.getGenericSequence();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return internal::flattenOptional(ranges::view::transform(generic(val), p2));
	}};
}

template <ranges::category range_cat>
void addProxyMapOptionalFuncsMethodsCore(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapOptional<range_cat, Block>)
	.def("_map", mapOptional<range_cat, Transaction>)
	.def("_map", mapOptional<range_cat, Input>)
	.def("_map", mapOptional<range_cat, Output>)
	.def("_map", mapOptional<range_cat, AnyScript>)
	.def("_map", mapOptional<range_cat, EquivAddress>)
	.def("_map", mapOptional<range_cat, Cluster>)
	.def("_map", mapOptional<range_cat, TaggedCluster>)
	.def("_map", mapOptional<range_cat, TaggedAddress>)
	;
}

template <ranges::category range_cat>
void addProxyMapOptionalFuncsMethodsScripts(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapOptional<range_cat, script::Pubkey>)
	.def("_map", mapOptional<range_cat, script::PubkeyHash>)
	.def("_map", mapOptional<range_cat, script::WitnessPubkeyHash>)
	.def("_map", mapOptional<range_cat, script::MultisigPubkey>)
	.def("_map", mapOptional<range_cat, script::Multisig>)
	.def("_map", mapOptional<range_cat, script::ScriptHash>)
	.def("_map", mapOptional<range_cat, script::WitnessScriptHash>)
	.def("_map", mapOptional<range_cat, script::OpReturn>)
	.def("_map", mapOptional<range_cat, script::Nonstandard>)
	;
}

template <ranges::category range_cat>
void addProxyMapOptionalFuncsMethodsOther(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapOptional<range_cat, AddressType::Enum>)
	.def("_map", mapOptional<range_cat, int64_t>)
	.def("_map", mapOptional<range_cat, bool>)
	.def("_map", mapOptional<range_cat, std::chrono::system_clock::time_point>)
	.def("_map", mapOptional<range_cat, uint256>)
	.def("_map", mapOptional<range_cat, uint160>)
	.def("_map", mapOptional<range_cat, pybind11::bytes>)
	.def("_map", mapOptional<range_cat, pybind11::list>)
	.def("_map", mapOptional<range_cat, std::string>)
	;
}

#endif /* proxy_range_map_optional_impl_hpp */
