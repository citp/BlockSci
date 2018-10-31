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

void applyProxyMapOptionalFuncsOther(pybind11::class_<RangeProxy> &cl) {
	addProxyMapOptionalFuncsMethodsOther<random_access_sized>(cl);
}

void applyProxyMapOptionalFuncsOther(pybind11::class_<IteratorProxy> &cl) {
	addProxyMapOptionalFuncsMethodsOther<ranges::category::input>(cl);
}
