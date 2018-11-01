//
//  python_proxies.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "generic_proxy/range.hpp"
#include "generic_proxy/optional.hpp"

namespace py = pybind11;
using namespace blocksci;

void setupProxies(py::module &m) {
	auto proxyMod = m.def_submodule("proxy");

    py::enum_<ProxyType>(proxyMod, "proxy_type", py::arithmetic(), "Enumeration of all proxy types")
    .value("simple", ProxyType::Simple)
    .value("optional", ProxyType::Optional)
    .value("iterator", ProxyType::Iterator)
    .value("range", ProxyType::Range)
    .def("__str__", [](ProxyType val) {
        switch (val) {
            case ProxyType::Simple:
                return "Simple";
            case ProxyType::Optional:
                return "Optional";
            case ProxyType::Iterator:
                return "Iterator";
            case ProxyType::Range:
                return "Range";
            default:
                return "Unknown Proxy Type";
        }
    })
    ;

    py::class_<GenericProxy> proxyCl(proxyMod, "Proxy");

    py::class_<SimpleProxy> proxySimpleCl(proxyMod, "SimpleProxy", proxyCl);
    proxySimpleCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Simple;
    })
    ;
        
    py::class_<OptionalProxy> proxyOptionalCl(proxyMod, "OptionalProxy", proxyCl);
    proxyOptionalCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Optional;
    })
    ;

    py::class_<IteratorProxy> proxyIteratorCl(proxyMod, "IteratorProxy", proxyCl);
    proxyIteratorCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Iterator;
    })
    ;

    py::class_<RangeProxy> proxyRangeCl(proxyMod, "RangeProxy", proxyIteratorCl);
    proxyRangeCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Range;
    })
    ;

    MainProxies mainProxies(proxyMod, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl);
    ScriptProxies scriptProxies(proxyMod, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl);
    OtherProxies otherProxies(proxyMod, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl);

    addOptionalProxyMethods(proxyOptionalCl);
    applyProxyIteratorFuncs(proxyIteratorCl);
    applyProxyRangeFuncs(proxyRangeCl);

    setupMainProxies(mainProxies);
    setupScriptProxies(scriptProxies);
    setupOtherProxies(otherProxies);
}