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

OtherProxies::OtherProxies(py::module &m, py::class_<SimpleProxy, GenericProxy> &proxySimpleCl) :
addressType(m, proxySimpleCl),
integer(m, proxySimpleCl),
boolean(m, proxySimpleCl),
time(m, proxySimpleCl),
uint256(m, proxySimpleCl),
uint160(m, proxySimpleCl),
string(m, proxySimpleCl),
bytes(m, proxySimpleCl),
list(m, proxySimpleCl) {}


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