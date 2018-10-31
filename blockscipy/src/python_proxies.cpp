//
//  python_proxies.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "proxy/range_map.hpp"

namespace py = pybind11;
using namespace blocksci;

void setupProxies(py::module &m) {
	auto proxyMod = m.def_submodule("proxy");
    py::class_<IteratorProxy> proxyIteratorCl(proxyMod, "IteratorProxy");
    py::class_<RangeProxy> proxyRangeCl(proxyMod, "RangeProxy", proxyIteratorCl);

    MainProxies mainProxies(proxyMod, proxyIteratorCl, proxyRangeCl);
    ScriptProxies scriptProxies(proxyMod, proxyIteratorCl, proxyRangeCl);
    OtherProxies otherProxies(proxyMod, proxyIteratorCl, proxyRangeCl);

    applyProxyMapFuncs(proxyIteratorCl);
    applyProxyMapOptionalFuncs(proxyIteratorCl);
    applyProxyMapSequenceFuncs(proxyIteratorCl);

    applyProxyMapFuncs(proxyRangeCl);
    applyProxyMapOptionalFuncs(proxyRangeCl);
    applyProxyMapSequenceFuncs(proxyRangeCl);

    setupMainProxies(mainProxies);
    setupScriptProxies(scriptProxies);
    setupOtherProxies(otherProxies);
}