//
//  range_map_sequence_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_sequence_impl_hpp
#define proxy_range_map_sequence_impl_hpp

#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

template<ranges::category range_cat, typename R>
Proxy<Iterator<R>> mapIterator(ProxySequence<range_cat> &seq, Proxy<Iterator<R>> &p2) {
	auto generic = seq.getGenericSequence();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return ranges::view::join(ranges::view::transform(generic(val), p2));
	}};
}

template<ranges::category range_cat, typename R>
Proxy<Iterator<R>> mapRange(ProxySequence<range_cat> &seq, Proxy<Range<R>> &p2) {
	auto generic = seq.getGenericSequence();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return ranges::view::join(ranges::view::transform(generic(val), p2));
	}};
}

template <ranges::category range_cat>
void addProxyMapIteratorFuncsMethods(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapIterator<range_cat, Block>)
	.def("_map", mapIterator<range_cat, Transaction>)
	.def("_map", mapIterator<range_cat, Input>)
	.def("_map", mapIterator<range_cat, Output>)
	.def("_map", mapIterator<range_cat, AnyScript>)
	.def("_map", mapIterator<range_cat, EquivAddress>)

	.def("_map", mapIterator<range_cat, Cluster>)
	.def("_map", mapIterator<range_cat, TaggedCluster>)
	.def("_map", mapIterator<range_cat, TaggedAddress>)
	;
}

template <ranges::category range_cat>
void addProxyMapRangeFuncsMethods(pybind11::class_<ProxySequence<range_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapRange<range_cat, Block>)
	.def("_map", mapRange<range_cat, Transaction>)
	.def("_map", mapRange<range_cat, Input>)
	.def("_map", mapRange<range_cat, Output>)
	.def("_map", mapRange<range_cat, AnyScript>)
	.def("_map", mapRange<range_cat, EquivAddress>)

	.def("_map", mapRange<range_cat, Cluster>)
	.def("_map", mapRange<range_cat, TaggedCluster>)
	.def("_map", mapRange<range_cat, TaggedAddress>)
	;
}


#endif /* proxy_range_map_optional_hpp */
