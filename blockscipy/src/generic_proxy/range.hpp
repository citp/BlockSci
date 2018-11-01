//
//  range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//


#ifndef generic_proxy_range_hpp
#define generic_proxy_range_hpp

#include "proxy.hpp"

#include <pybind11/pybind11.h>

void applyProxyIteratorFuncs(pybind11::class_<IteratorProxy> &cl);
void applyProxyRangeFuncs(pybind11::class_<RangeProxy> &cl);


#endif /* generic_proxy_range_hpp */
