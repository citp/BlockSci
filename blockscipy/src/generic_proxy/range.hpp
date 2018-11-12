//
//  range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//


#ifndef generic_proxy_range_hpp
#define generic_proxy_range_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void applyProxyIteratorFuncs(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyRangeFuncs(pybind11::class_<RangeProxy, IteratorProxy> &cl);


#endif /* generic_proxy_range_hpp */
