//
//  python_proxies.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef python_proxies_h
#define python_proxies_h

#include "proxy.hpp"

#include <pybind11/pybind11.h>

void setupMainProxies(pybind11::module &m, pybind11::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, pybind11::class_<ProxySequence<random_access_sized>> &proxyRangeCl);
void setupScriptProxies(pybind11::module &m, pybind11::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, pybind11::class_<ProxySequence<random_access_sized>> &proxyRangeCl);
void setupOtherProxies(pybind11::module &m, pybind11::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, pybind11::class_<ProxySequence<random_access_sized>> &proxyRangeCl);

void setupProxies(pybind11::module &m);

#endif /* python_proxies_h */