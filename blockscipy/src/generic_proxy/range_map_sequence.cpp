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

void applyProxyMapSequenceFuncs(pybind11::class_<IteratorProxy, GenericProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_sequence", mapSequence<Block>)
	.def("_map_sequence", mapSequence<Transaction>)
	.def("_map_sequence", mapSequence<Input>)
	.def("_map_sequence", mapSequence<Output>)
	.def("_map_sequence", mapSequence<AnyScript>)
	.def("_map_sequence", mapSequence<EquivAddress>)

	.def("_map_sequence", mapSequence<Cluster>)
	.def("_map_sequence", mapSequence<TaggedCluster>)
	.def("_map_sequence", mapSequence<TaggedAddress>)
	;
}