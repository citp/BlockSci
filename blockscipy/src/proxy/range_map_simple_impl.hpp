//
//  range_map_simple_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_simple_impl_hpp
#define proxy_range_map_simple_impl_hpp

#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/transform.hpp>

template<ranges::category range_cat, typename R>
Proxy<any_view<R, range_cat>> mapSimple(ProxySequence<range_cat> &seq, Proxy<R> &p2) {
	auto generic = seq.getGenericSequence();
	return std::function<any_view<R, range_cat>(std::any &)>{[=](std::any &val) -> any_view<R, range_cat> {
		return ranges::view::transform(generic(val), [=](std::any v) {
			return p2(v);
		});
	}};
}

template <ranges::category range_cat>
void addProxyMapFuncsMethodsCore(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("map", mapSimple<range_cat, Block>)
	.def("map", mapSimple<range_cat, Transaction>)
	.def("map", mapSimple<range_cat, Input>)
	.def("map", mapSimple<range_cat, Output>)
	.def("map", mapSimple<range_cat, AnyScript>)
	.def("map", mapSimple<range_cat, EquivAddress>)
	.def("map", mapSimple<range_cat, Cluster>)
	.def("map", mapSimple<range_cat, TaggedCluster>)
	.def("map", mapSimple<range_cat, TaggedAddress>)
	;
}

template <ranges::category range_cat>
void addProxyMapFuncsMethodsScripts(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("map", mapSimple<range_cat, script::Pubkey>)
	.def("map", mapSimple<range_cat, script::PubkeyHash>)
	.def("map", mapSimple<range_cat, script::WitnessPubkeyHash>)
	.def("map", mapSimple<range_cat, script::MultisigPubkey>)
	.def("map", mapSimple<range_cat, script::Multisig>)
	.def("map", mapSimple<range_cat, script::ScriptHash>)
	.def("map", mapSimple<range_cat, script::WitnessScriptHash>)
	.def("map", mapSimple<range_cat, script::OpReturn>)
	.def("map", mapSimple<range_cat, script::Nonstandard>)
	;
}

template <ranges::category range_cat>
void addProxyMapFuncsMethodsOther(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("map", mapSimple<range_cat, AddressType::Enum>)
	.def("map", mapSimple<range_cat, int64_t>)
	.def("map", mapSimple<range_cat, bool>)
	.def("map", mapSimple<range_cat, std::chrono::system_clock::time_point>)
	.def("map", mapSimple<range_cat, uint256>)
	.def("map", mapSimple<range_cat, uint160>)
	.def("map", mapSimple<range_cat, pybind11::bytes>)
	.def("map", mapSimple<range_cat, pybind11::list>)
	.def("map", mapSimple<range_cat, std::string>)
	;
}

#endif /* proxy_range_map_simple_impl_hpp */
