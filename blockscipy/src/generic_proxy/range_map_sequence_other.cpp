//
//  range_map_sequence_other.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//
//

#include "range_map.hpp"
#include "range_map_sequence_impl.hpp"

void applyProxyMapSequenceFuncsOther(pybind11::class_<IteratorProxy, GenericProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_sequence", mapSequence<AddressType::Enum>)
	.def("_map_sequence", mapSequence<int64_t>)
	.def("_map_sequence", mapSequence<bool>)
	.def("_map_sequence", mapSequence<std::chrono::system_clock::time_point>)
	.def("_map_sequence", mapSequence<uint256>)
	.def("_map_sequence", mapSequence<uint160>)
	.def("_map_sequence", mapSequence<pybind11::bytes>)
	.def("_map_sequence", mapSequence<pybind11::list>)
	.def("_map_sequence", mapSequence<std::string>)
	;
}
