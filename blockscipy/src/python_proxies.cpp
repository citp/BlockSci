//
//  python_proxies.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "caster_py.hpp"
#include "proxy_py.hpp"
#include "proxy/range_map.hpp"

namespace py = pybind11;
using namespace blocksci;

void setupProxies(py::module &m) {
	auto proxyMod = m.def_submodule("proxy");
    py::class_<ProxySequence<ranges::category::input>> proxyIteratorCl(proxyMod, "ProxyIterator");
    py::class_<ProxySequence<random_access_sized>> proxyRangeCl(proxyMod, "ProxyRange");

    applyProxyMapFuncs(proxyIteratorCl);
    applyProxyMapFuncs(proxyRangeCl);

    applyProxyMapOptionalFuncs(proxyIteratorCl);
    applyProxyMapOptionalFuncs(proxyRangeCl);

    applyProxyMapSequenceFuncs(proxyIteratorCl);
    applyProxyMapSequenceFuncs(proxyRangeCl);

    setupMainProxies(proxyMod, proxyIteratorCl, proxyRangeCl);
    setupScriptProxies(proxyMod, proxyIteratorCl, proxyRangeCl);
    setupOtherProxies(proxyMod, proxyIteratorCl, proxyRangeCl);
}