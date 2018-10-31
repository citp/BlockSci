//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_hpp
#define proxy_range_map_hpp

#include "proxy_py.hpp"

void applyProxyMapOptionalFuncsCore(pybind11::class_<ProxyIterator> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<ProxyIterator> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<ProxyIterator> &cl);

void applyProxyMapOptionalFuncsCore(pybind11::class_<ProxyRange> &cl);
void applyProxyMapOptionalFuncsScripts(pybind11::class_<ProxyRange> &cl);
void applyProxyMapOptionalFuncsOther(pybind11::class_<ProxyRange> &cl);


void applyProxyMapFuncsCore(pybind11::class_<ProxyIterator> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<ProxyIterator> &cl);
void applyProxyMapFuncsOther(pybind11::class_<ProxyIterator> &cl);

void applyProxyMapFuncsCore(pybind11::class_<ProxyRange> &cl);
void applyProxyMapFuncsScripts(pybind11::class_<ProxyRange> &cl);
void applyProxyMapFuncsOther(pybind11::class_<ProxyRange> &cl);


inline void applyProxyMapFuncs(pybind11::class_<ProxyRange> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapFuncs(pybind11::class_<ProxyIterator> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);
}

inline void applyProxyMapOptionalFuncs(pybind11::class_<ProxyRange> &cl) {
	applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);
}

inline void applyProxyMapOptionalFuncs(pybind11::class_<ProxyIterator> &cl) {
	applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);
}

void applyProxyMapSequenceFuncs(pybind11::class_<ProxyIterator> &cl);
void applyProxyMapSequenceFuncs(pybind11::class_<ProxyRange> &cl);


#endif /* proxy_range_map_hpp */
