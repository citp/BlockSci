//
//  python_proxies_other.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "proxy_py_create.hpp"
#include "sequence.hpp"
#include "simple/simple_proxies.hpp"

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

OtherProxies::OtherProxies(py::module &m) :
uint256(createProxyClasses<blocksci::uint256>(m)),
uint160(createProxyClasses<blocksci::uint160>(m)),
addressType(createProxyClasses<AddressType::Enum>(m)),
integer(createProxyClasses<int64_t>(m)),
boolean(createProxyClasses<bool>(m)),
time(createProxyClasses<std::chrono::system_clock::time_point>(m)),
string(createProxyClasses<std::string>(m)),
bytes(createProxyClasses<py::bytes>(m)),
list(createProxyClasses<py::list>(m)) {}


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