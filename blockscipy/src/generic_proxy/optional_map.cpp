//
//  optional_map.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/31/18.
//
//

#include "optional_map.hpp"

#include <blocksci/chain/block.hpp>

#include <chrono>

using namespace blocksci;


void addOptionalProxyMapMethods(pybind11::class_<OptionalProxy, GenericProxy> &cl) {
	cl
	.def("_map", mapOptional<Block>)
	.def("_map", mapOptional<Transaction>)
	.def("_map", mapOptional<Input>)
	.def("_map", mapOptional<Output>)
	.def("_map", mapOptional<AnyScript>)
	.def("_map", mapOptional<AddressType::Enum>)
	.def("_map", mapOptional<int64_t>)
	.def("_map", mapOptional<bool>)
	.def("_map", mapOptional<std::chrono::system_clock::time_point>)
	.def("_map", mapOptional<uint256>)
	.def("_map", mapOptional<uint160>)
	.def("_map", mapOptional<pybind11::bytes>)
	;
}