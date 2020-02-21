//
//  proxy_py_create.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/13/18.
//

#ifndef proxy_py_create_hpp
#define proxy_py_create_hpp

#include "proxy.hpp"
#include "proxy_type_check.hpp"
#include "method_types.hpp"

#include <range/v3/view/empty.hpp>
#include <range/v3/view/single.hpp>

template <typename To>
std::string proxyName() {
    return typenameLookup().getName<To>() + "Proxy";
}

template <typename To>
std::string proxySequenceName() {
    return typenameLookup().getName<To>() + "SequenceProxy";
}

template <typename T, typename SimpleBase = SimpleProxy>
AllProxyClasses<T, SimpleBase> createProxyClasses(pybind11::module &m) {
    pybind11::class_<Proxy<T>, SimpleBase> base(m, strdup(proxyName<T>().c_str()));
    pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> optional(m, strdup(proxyName<ranges::optional<T>>().c_str()));
    pybind11::class_<SequenceProxy<T>> sequence(m, strdup(proxySequenceName<T>().c_str()));
    pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>> iterator(m, strdup(proxyName<Iterator<T>>().c_str()), pybind11::dynamic_attr());
    pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>> range(m, strdup(proxyName<Range<T>>().c_str()), pybind11::dynamic_attr());

    base.def(pybind11::init([](const T &val) -> Proxy<T> {
        return {std::function<T(std::any &)>{[val](std::any &) -> T {
            return val;
        }}, {nullptr, nullptr, ProxyType::Simple}};
    }));

    iterator
    .def(pybind11::init([](const Iterator<T> &val) -> Proxy<RawIterator<T>> {
        return {std::function<RawIterator<T>(std::any &)>{[r = val.rng](std::any &) -> RawIterator<T> {
            return r;
        }}, {nullptr, nullptr, ProxyType::Simple}};
    }))
    .def(pybind11::init([](const Proxy<RawRange<T>> &p) -> Proxy<RawIterator<T>> {
        return {std::function<RawIterator<T>(std::any &)>{[p](std::any & v) -> RawIterator<T> {
            return p(v);
        }}, p.sourceType};
    }))
    ;

    range.def(pybind11::init([](const Range<T> &val) -> Proxy<RawRange<T>> {
        return {std::function<RawRange<T>(std::any &)>{[r = val.rng](std::any &) -> RawRange<T> {
            return r;
        }}, {nullptr, nullptr, ProxyType::Simple}};
    }))
    .def(pybind11::init([](const Proxy<ranges::optional<T>> &p) -> Proxy<RawRange<T>> {
        return {std::function<RawRange<T>(std::any &)>{[p](std::any & v) -> RawRange<T> {
            auto val = p(v);
            if (val) {
                return ranges::views::single(*val);
            } else {
                return ranges::views::empty<T>;
            }
        }}, p.sourceType};
    }))
    ;

    optional
    .def(pybind11::init([](const Proxy<T> &p) -> Proxy<ranges::optional<T>> {
        return {std::function<ranges::optional<T>(std::any &)>{[p](std::any &v) -> ranges::optional<T> {
            return p(v);
        }}, p.sourceType};
    }))
    ;

    pybind11::implicitly_convertible<T, Proxy<T>>();
    pybind11::implicitly_convertible<T, Proxy<ranges::optional<T>>>();
    pybind11::implicitly_convertible<T, Proxy<RawIterator<T>>>();
    pybind11::implicitly_convertible<T, Proxy<RawRange<T>>>();

    pybind11::implicitly_convertible<Range<T>, Proxy<RawIterator<T>>>();
    pybind11::implicitly_convertible<Range<T>, Proxy<RawRange<T>>>();

    pybind11::implicitly_convertible<Iterator<T>, Proxy<RawIterator<T>>>();

    pybind11::implicitly_convertible<Proxy<T>, Proxy<ranges::optional<T>>>();
    pybind11::implicitly_convertible<Proxy<T>, Proxy<RawIterator<T>>>();
    pybind11::implicitly_convertible<Proxy<T>, Proxy<RawRange<T>>>();

    pybind11::implicitly_convertible<Proxy<ranges::optional<T>>, Proxy<RawRange<T>>>();
    pybind11::implicitly_convertible<Proxy<ranges::optional<T>>, Proxy<RawIterator<T>>>();

    pybind11::implicitly_convertible<Proxy<RawRange<T>>, Proxy<RawIterator<T>>>();
    
    return {base, optional, sequence, iterator, range};
}

#endif /* proxy_py_create_hpp */
