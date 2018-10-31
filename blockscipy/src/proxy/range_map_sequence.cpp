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

void applyProxyMapSequenceFuncs(pybind11::class_<IteratorProxy> &cl) {
	addProxyMapSequenceFuncsMethods<random_access_sized, ranges::category::input>(cl);
	addProxyMapSequenceFuncsMethods<ranges::category::input, ranges::category::input>(cl);
}


void applyProxyMapSequenceFuncs(pybind11::class_<RangeProxy> &cl) {
	addProxyMapSequenceFuncsMethods<random_access_sized, random_access_sized>(cl);
	addProxyMapSequenceFuncsMethods<ranges::category::input, random_access_sized>(cl);
}
