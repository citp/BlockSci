//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "proxy_py.hpp"

void applyProxyMapOptionalFuncsCore(pybind11::class_<IteratorProxy> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<IteratorProxy> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<IteratorProxy> &cl);

void applyProxyMapFuncsCore(pybind11::class_<IteratorProxy> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<IteratorProxy> &cl);
void applyProxyMapFuncsOther(pybind11::class_<IteratorProxy> &cl);

void applyProxyMapFuncsCore(pybind11::class_<RangeProxy> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<RangeProxy> &cl);
void applyProxyMapFuncsOther(pybind11::class_<RangeProxy> &cl);

void addProxyIteratorMethods(pybind11::class_<IteratorProxy> &cl);
void addProxyRangeMethods(pybind11::class_<RangeProxy> &cl);

inline void applyProxyMapFuncs(pybind11::class_<RangeProxy> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapFuncs(pybind11::class_<IteratorProxy> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapOptionalFuncs(pybind11::class_<IteratorProxy> &cl) {
	applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);
}

void applyProxyMapSequenceFuncs(pybind11::class_<IteratorProxy> &cl);


#endif /* proxy_range_map_hpp */
