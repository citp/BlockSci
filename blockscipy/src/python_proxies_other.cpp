//
//  python_proxies_other.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "simple/simple_proxies.hpp"

namespace py = pybind11;
using namespace blocksci;

OtherProxies::OtherProxies(py::module &m, py::class_<SimpleProxy> &proxySimpleCl, py::class_<OptionalProxy> &proxyOptionalCl, py::class_<IteratorProxy> &proxyIteratorCl, py::class_<RangeProxy> &proxyRangeCl) :
addressType(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
integer(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
boolean(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
time(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
uint256(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
uint160(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
string(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
bytes(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
list(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl) {}


void setupOtherProxies(OtherProxies &proxies) {
    addAddressTypeProxyMethods(proxies.addressType);
    addIntProxyMethods(proxies.integer);
    addBoolProxyMethods(proxies.boolean);
    addTimeProxyMethods(proxies.time);
    addUint256ProxyMethods(proxies.uint256);
    addUint160ProxyMethods(proxies.uint160);
    addBytesProxyMethods(proxies.bytes);
    addStringProxyMethods(proxies.string);
    addListProxyMethods(proxies.list);
}