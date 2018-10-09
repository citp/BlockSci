//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "proxy_py.hpp"

void applyProxyMapOptionalFuncsCore(pybind11::class_<ProxySequence<ranges::category::input>> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<ProxySequence<ranges::category::input>> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<ProxySequence<ranges::category::input>> &cl);

void applyProxyMapOptionalFuncsCore(pybind11::class_<ProxySequence<random_access_sized>> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<ProxySequence<random_access_sized>> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<ProxySequence<random_access_sized>> &cl);


void applyProxyMapFuncsCore(pybind11::class_<ProxySequence<ranges::category::input>> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<ProxySequence<ranges::category::input>> &cl);
void applyProxyMapFuncsOther(pybind11::class_<ProxySequence<ranges::category::input>> &cl);

void applyProxyMapFuncsCore(pybind11::class_<ProxySequence<random_access_sized>> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<ProxySequence<random_access_sized>> &cl);
void applyProxyMapFuncsOther(pybind11::class_<ProxySequence<random_access_sized>> &cl);


inline void applyProxyMapFuncs(pybind11::class_<ProxySequence<random_access_sized>> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapFuncs(pybind11::class_<ProxySequence<ranges::category::input>> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapOptionalFuncs(pybind11::class_<ProxySequence<random_access_sized>> &cl) {
	applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);
}

inline void applyProxyMapOptionalFuncs(pybind11::class_<ProxySequence<ranges::category::input>> &cl) {
	applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);
}

void applyProxyMapSequenceFuncs(pybind11::class_<ProxySequence<random_access_sized>> &cl);
void applyProxyMapSequenceFuncs(pybind11::class_<ProxySequence<ranges::category::input>> &cl);


#endif /* proxy_range_map_hpp */
