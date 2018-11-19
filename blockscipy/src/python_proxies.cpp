//
//  python_proxies.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "proxy/proxy_functions.hpp"
#include "generic_proxy/range.hpp"
#include "generic_proxy/optional.hpp"
#include "method_types.hpp"

#include <blocksci/chain/block.hpp>

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

    py::class_<SimpleProxy, GenericProxy> proxySimpleCl(proxyMod, "SimpleProxy");
    proxySimpleCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Simple;
    })
    ;
        
    py::class_<OptionalProxy, GenericProxy> proxyOptionalCl(proxyMod, "OptionalProxy");
    proxyOptionalCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Optional;
    })
    ;

    py::class_<IteratorProxy, GenericProxy> proxyIteratorCl(proxyMod, "IteratorProxy");
    proxyIteratorCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Iterator;
    })
    ;

    py::class_<RangeProxy, IteratorProxy> proxyRangeCl(proxyMod, "RangeProxy");
    proxyRangeCl
    .def_property_readonly_static("ptype", [](pybind11::object &) -> ProxyType {
        return ProxyType::Range;
    })
    ;

    MainProxies mainProxies(proxyMod);
    ScriptProxies scriptProxies(proxyMod);
    OtherProxies otherProxies(proxyMod);

    defineProxyFunctions(m, proxyMod);

    addOptionalProxyMethods(proxyOptionalCl, m);
    applyProxyIteratorFuncs(proxyIteratorCl);
    applyProxyRangeFuncs(proxyRangeCl);

    setupMainProxies(mainProxies);
    setupScriptProxies(scriptProxies);
    setupOtherProxies(otherProxies);
}