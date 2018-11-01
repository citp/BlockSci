//
//  range_map_optional_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "range_map.hpp"
#include "range_map_optional_impl.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/cluster/cluster.hpp>

void applyProxyMapOptionalFuncsOther(pybind11::class_<IteratorProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_optional", mapOptional<AddressType::Enum>)
	.def("_map_optional", mapOptional<int64_t>)
	.def("_map_optional", mapOptional<bool>)
	.def("_map_optional", mapOptional<std::chrono::system_clock::time_point>)
	.def("_map_optional", mapOptional<uint256>)
	.def("_map_optional", mapOptional<uint160>)
	.def("_map_optional", mapOptional<pybind11::bytes>)
	.def("_map_optional", mapOptional<pybind11::list>)
	.def("_map_optional", mapOptional<std::string>)
	;
}
