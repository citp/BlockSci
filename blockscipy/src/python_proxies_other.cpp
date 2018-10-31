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

OtherProxies::OtherProxies(py::module &m, py::class_<ProxyIterator> &proxyIteratorCl, py::class_<ProxyRange> &proxyRangeCl) :
addressType(m, proxyIteratorCl, proxyRangeCl),
integer(m, proxyIteratorCl, proxyRangeCl),
boolean(m, proxyIteratorCl, proxyRangeCl),
time(m, proxyIteratorCl, proxyRangeCl),
uint256(m, proxyIteratorCl, proxyRangeCl),
uint160(m, proxyIteratorCl, proxyRangeCl),
string(m, proxyIteratorCl, proxyRangeCl),
bytes(m, proxyIteratorCl, proxyRangeCl),
list(m, proxyIteratorCl, proxyRangeCl) {}


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