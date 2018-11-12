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

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

OtherProxies::OtherProxies(py::module &m) :
addressType(m),
integer(m),
boolean(m),
time(m),
uint256(m),
uint160(m),
string(m),
bytes(m),
list(m) {}


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