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

template <typename To>
std::string proxySequenceName() {
    return PythonTypeName<To>::name() + "SequenceProxy";
}

template <typename T>
void addProxyConditional(pybind11::module &m) {
    m
    .def("conditional", [](const Proxy<bool> &cond, const Proxy<T> &p1, const Proxy<T> &p2) -> Proxy<T> {
        return std::function<T(std::any &)>{[cond, p1, p2](std::any &t) -> T {
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
private:
    void commonInit(pybind11::module &m) {
        base.def(pybind11::init<T>());
        iterator.def(pybind11::init<Iterator<T>>());
        range.def(pybind11::init<Range<T>>());

        optional.def(pybind11::init<Proxy<T>>());
        iterator.def(pybind11::init<Proxy<RawRange<T>>>());

        pybind11::implicitly_convertible<T, Proxy<T>>();
        pybind11::implicitly_convertible<Iterator<T>, Proxy<RawIterator<T>>>();
        pybind11::implicitly_convertible<Range<T>, Proxy<RawRange<T>>>();

        pybind11::implicitly_convertible<Proxy<T>, Proxy<ranges::optional<T>>>();
        pybind11::implicitly_convertible<Proxy<RawRange<T>>, Proxy<RawIterator<T>>>();


        addProxyConditional<T>(m);
        addProxyConditional<ranges::optional<T>>(m);
        addProxyConditional<Iterator<T>>(m);
        addProxyConditional<Range<T>>(m);
    }
public:
    pybind11::class_<Proxy<T>> base;
    pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> optional;
    pybind11::class_<SequenceProxy<T>> sequence;
    pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>> iterator;
    pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>> range;

	AllProxyClasses(
        pybind11::module &m,
        pybind11::class_<SimpleProxy, GenericProxy> &proxySimpleCl) : 
    	base(m, strdup(proxyName<T>().c_str()), proxySimpleCl),
    	optional(m, strdup(proxyName<ranges::optional<T>>().c_str())),
        sequence(m, strdup(proxySequenceName<T>().c_str())),
    	iterator(m, strdup(proxyName<Iterator<T>>().c_str())),
    	range(m, strdup(proxyName<Range<T>>().c_str())) {
        commonInit(m);
    }

    AllProxyClasses(
        pybind11::module &m,
        pybind11::class_<ProxyAddress> proxyAddressCl) : 
        base(m, strdup(proxyName<T>().c_str()), proxyAddressCl),
        optional(m, strdup(proxyName<ranges::optional<T>>().c_str())),
        sequence(m, strdup(proxySequenceName<T>().c_str())),
        iterator(m, strdup(proxyName<Iterator<T>>().c_str())),
        range(m, strdup(proxyName<Range<T>>().c_str())) {
        commonInit(m);
    }

	template<typename Func>
    void applyToAll(Func func) {
    	func(base);
        func(optional);
        func(iterator);
        func(range);
    }
};

#endif /* proxy_py_hpp */


