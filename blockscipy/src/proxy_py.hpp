//
//  proxy_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_py_hpp
#define proxy_py_hpp

#include "proxy.hpp"
#include "generic_sequence.hpp"
#include "method_types.hpp"

#include <pybind11/pybind11.h>

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
struct AllProxyClasses {
    pybind11::class_<Proxy<T>> base;
    pybind11::class_<Proxy<ranges::optional<T>>> optional;
    pybind11::class_<Proxy<Iterator<T>>> iterator;
    pybind11::class_<Proxy<Range<T>>> range;

	AllProxyClasses(
        pybind11::module &m,
        pybind11::class_<SimpleProxy> &proxySimpleCl,
        pybind11::class_<OptionalProxy> &proxyOptionalCl,
        pybind11::class_<IteratorProxy> proxyIteratorCl,
        pybind11::class_<RangeProxy> proxyRangeCl) : 
    	base(m, strdup(proxyName<T>().c_str()), proxySimpleCl),
    	optional(m, strdup(proxyName<ranges::optional<T>>().c_str()), proxyOptionalCl),
    	iterator(m, strdup(proxyName<Iterator<T>>().c_str()), proxyIteratorCl),
    	range(m, strdup(proxyName<Range<T>>().c_str()), proxyRangeCl) {
        base.def(pybind11::init<T>());
        optional.def(pybind11::init<T>());
        optional.def(pybind11::init<Proxy<T>>());
        iterator.def(pybind11::init<Iterator<T>>());
        range.def(pybind11::init<Range<T>>());

        pybind11::implicitly_convertible<T, Proxy<T>>();
        pybind11::implicitly_convertible<T, Proxy<ranges::optional<T>>>();
        pybind11::implicitly_convertible<Proxy<T>, Proxy<ranges::optional<T>>>();
        pybind11::implicitly_convertible<Iterator<T>, Proxy<Iterator<T>>>();
        pybind11::implicitly_convertible<Range<T>, Proxy<Range<T>>>();

        addProxyConditional<T>(m);
        addProxyConditional<ranges::optional<T>>(m);
        addProxyConditional<Iterator<T>>(m);
        addProxyConditional<Range<T>>(m);
    }

    AllProxyClasses(
        pybind11::module &m,
        pybind11::class_<ProxyAddress> proxyAddressCl,
        pybind11::class_<OptionalProxy> &proxyOptionalCl,
        pybind11::class_<IteratorProxy> proxyIteratorCl,
        pybind11::class_<RangeProxy> proxyRangeCl) : 
        base(m, strdup(proxyName<T>().c_str()), proxyAddressCl),
        optional(m, strdup(proxyName<ranges::optional<T>>().c_str()), proxyOptionalCl),
        iterator(m, strdup(proxyName<Iterator<T>>().c_str()), proxyIteratorCl),
        range(m, strdup(proxyName<Range<T>>().c_str()), proxyRangeCl
    ) {
        base.def(pybind11::init<T>());
        optional.def(pybind11::init<T>());
        optional.def(pybind11::init<Proxy<T>>());
        iterator.def(pybind11::init<Iterator<T>>());
        range.def(pybind11::init<Range<T>>());

        pybind11::implicitly_convertible<T, Proxy<T>>();
        pybind11::implicitly_convertible<T, Proxy<ranges::optional<T>>>();
        pybind11::implicitly_convertible<Proxy<T>, Proxy<ranges::optional<T>>>();
        pybind11::implicitly_convertible<Iterator<T>, Proxy<Iterator<T>>>();
        pybind11::implicitly_convertible<Range<T>, Proxy<Range<T>>>();

        addProxyConditional<T>(m);
        addProxyConditional<ranges::optional<T>>(m);
        addProxyConditional<Iterator<T>>(m);
        addProxyConditional<Range<T>>(m);
    }

	template<typename Func>
    void applyToAll(Func func) {
    	func(base);
        func(optional);
        func(iterator);
        func(range);
    }

    template<typename Func>
    void applyToRanges(Func func) {
    	func(iterator);
    	func(range);
    }

    template<typename Func>
    void setupSimpleProxy(Func func) {
    	func(base);
    	func(optional);
    }
};

#endif /* proxy_py_hpp */


