//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void applyProxyMapOptionalFuncsCore(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<IteratorProxy, GenericProxy> &cl);

void applyProxyMapFuncsCore(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapFuncsOther(pybind11::class_<IteratorProxy, GenericProxy> &cl);

void applyProxyMapFuncsCore(pybind11::class_<RangeProxy, IteratorProxy> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<RangeProxy, IteratorProxy> &cl);
void applyProxyMapFuncsOther(pybind11::class_<RangeProxy, IteratorProxy> &cl);

void applyProxyMapSequenceFuncsCore(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapSequenceFuncsScripts(pybind11::class_<IteratorProxy, GenericProxy> &cl);
void applyProxyMapSequenceFuncsOther(pybind11::class_<IteratorProxy, GenericProxy> &cl);

#endif /* proxy_range_map_hpp */
