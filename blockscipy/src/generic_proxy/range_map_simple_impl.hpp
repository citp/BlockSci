//
//  range_map_simple_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_simple_impl_hpp
#define proxy_range_map_simple_impl_hpp

#include "proxy.hpp"
#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/transform.hpp>

template<ranges::category range_cat, typename R>
Proxy<ranges::any_view<R, range_cat>> mapSimple(proxy_sequence<range_cat> &p, Proxy<R> &p2) {
	return liftGeneric(p, [p2](auto && seq) -> ranges::any_view<R, range_cat> {
		return ranges::views::transform(std::forward<decltype(seq)>(seq).toAnySequence(), p2);
	});
}

template <ranges::category range_cat, typename Class>
void addProxyMapFuncsMethodsCore(Class &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapSimple<range_cat, Block>)
	.def("_map", mapSimple<range_cat, Transaction>)
	.def("_map", mapSimple<range_cat, Input>)
	.def("_map", mapSimple<range_cat, Output>)
	.def("_map", mapSimple<range_cat, AnyScript>)
	.def("_map", mapSimple<range_cat, EquivAddress>)
	.def("_map", mapSimple<range_cat, Cluster>)
	.def("_map", mapSimple<range_cat, TaggedCluster>)
	.def("_map", mapSimple<range_cat, TaggedAddress>)
	;
}

template <ranges::category range_cat, typename Class>
void addProxyMapFuncsMethodsScripts(Class &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapSimple<range_cat, script::Pubkey>)
	.def("_map", mapSimple<range_cat, script::PubkeyHash>)
	.def("_map", mapSimple<range_cat, script::WitnessPubkeyHash>)
	.def("_map", mapSimple<range_cat, script::MultisigPubkey>)
	.def("_map", mapSimple<range_cat, script::Multisig>)
	.def("_map", mapSimple<range_cat, script::ScriptHash>)
	.def("_map", mapSimple<range_cat, script::WitnessScriptHash>)
	.def("_map", mapSimple<range_cat, script::OpReturn>)
	.def("_map", mapSimple<range_cat, script::Nonstandard>)
	.def("_map", mapSimple<range_cat, script::WitnessUnknown>)
	;
}

template <ranges::category range_cat, typename Class>
void addProxyMapFuncsMethodsOther(Class &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapSimple<range_cat, AddressType::Enum>)
	.def("_map", mapSimple<range_cat, int64_t>)
	.def("_map", mapSimple<range_cat, bool>)
	.def("_map", mapSimple<range_cat, std::chrono::system_clock::time_point>)
	.def("_map", mapSimple<range_cat, uint256>)
	.def("_map", mapSimple<range_cat, uint160>)
	.def("_map", mapSimple<range_cat, pybind11::bytes>)
	.def("_map", mapSimple<range_cat, pybind11::list>)
	.def("_map", mapSimple<range_cat, std::string>)
	;
}

#endif /* proxy_range_map_simple_impl_hpp */
