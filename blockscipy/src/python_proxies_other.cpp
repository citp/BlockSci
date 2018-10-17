//
//  python_proxies_other.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "caster_py.hpp"
#include "proxy_py.hpp"
#include "simple/simple_proxies.hpp"

namespace py = pybind11;
using namespace blocksci;

void setupOtherProxies(py::module &m, py::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, py::class_<ProxySequence<random_access_sized>> &proxyRangeCl) {

    AllProxyClasses<AddressType::Enum> addressTypeProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<int64_t> intProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<bool> boolProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<std::chrono::system_clock::time_point> timeProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<uint256> uint256ProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<uint160> uint160ProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<std::string> stringProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<py::bytes> bytesProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<py::list> listProxyCls(m, proxyIteratorCl, proxyRangeCl);

    addAddressTypeProxyMethods(addressTypeProxyCls);
    addIntProxyMethods(intProxyCls);
    addBoolProxyMethods(boolProxyCls);
    addTimeProxyMethods(timeProxyCls);
    addUint256ProxyMethods(uint256ProxyCls);
    addUint160ProxyMethods(uint160ProxyCls);
    addBytesProxyMethods(bytesProxyCls);
    addStringProxyMethods(stringProxyCls);
    addListProxyMethods(listProxyCls);
}