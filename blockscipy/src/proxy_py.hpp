//
//  proxy_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_py_hpp
#define proxy_py_hpp

#include "proxy.hpp"
#include "proxy_utils.hpp"
#include "range_utils.hpp"
#include "method_types.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

#include <functional>

template <typename To>
std::string proxyName() {
	return PythonTypeName<To>::name() + "Proxy";
}

template <typename T>
void addProxyConditional(pybind11::module &m) {
    m
    .def("conditional", [](const Proxy<bool> &cond, const Proxy<T> &p1, const Proxy<T> &p2) -> Proxy<T> {
        return std::function<T(std::any &)>{[=](std::any &t) -> T {
            if(cond(t)) {
                return p1(t);
            } else {
                return p2(t);
            }
        }};
    })
    ;
}

template <typename T>
struct ProxyClasses {
	pybind11::class_<Proxy<T>> proxy;

    ProxyClasses(pybind11::module &m) : 
    proxy(m, strdup(proxyName<T>().c_str())) {

        addProxyConditional<T>(m);
    }

    template<typename Func>
    void applyToAll(Func func) {
        func(proxy);
    }
};

template <typename T>
struct AllProxyClasses {
	ProxyClasses<T> base;
	ProxyClasses<ranges::optional<T>> optional;
	ProxyClasses<Iterator<T>> iterator;
	ProxyClasses<Range<T>> range;

	AllProxyClasses(pybind11::module &m) : 
	base(m),
	optional(m),
	iterator(m),
	range(m) {}

	template<typename Func>
    void applyToAll(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }

    template<typename Func>
    void applyToRanges(Func func) {
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }
    
    template<typename Func>
    void setupBasicProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }

    template<typename Func>
    void setupSimpleProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    }
};

#endif /* proxy_py_hpp */


