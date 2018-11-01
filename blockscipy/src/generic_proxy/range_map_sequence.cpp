//
//  range_map_sequence.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "range_map.hpp"
#include "range_map_sequence_impl.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>


template <ranges::category range_cat>
void addProxyMapSequenceFuncsMethods(pybind11::class_<IteratorProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_sequence", mapSequence<range_cat, Block>)
	.def("_map_sequence", mapSequence<range_cat, Transaction>)
	.def("_map_sequence", mapSequence<range_cat, Input>)
	.def("_map_sequence", mapSequence<range_cat, Output>)
	.def("_map_sequence", mapSequence<range_cat, AnyScript>)
	.def("_map_sequence", mapSequence<range_cat, EquivAddress>)

	.def("_map_sequence", mapSequence<range_cat, Cluster>)
	.def("_map_sequence", mapSequence<range_cat, TaggedCluster>)
	.def("_map_sequence", mapSequence<range_cat, TaggedAddress>)
	;
}

void applyProxyMapSequenceFuncs(pybind11::class_<IteratorProxy> &cl) {
	addProxyMapSequenceFuncsMethods<random_access_sized>(cl);
	addProxyMapSequenceFuncsMethods<ranges::category::input>(cl);
}