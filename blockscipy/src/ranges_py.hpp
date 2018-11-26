//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "python_fwd.hpp"
#include "python_range.hpp"
#include "proxy.hpp"
#include "generic_sequence.hpp"
#include "proxy_create.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

template<typename Class>
auto addCommonIteratorMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__iter__", [](R &range) { 
        return pybind11::make_iterator(range.rng.begin(), range.rng.end()); 
    }, pybind11::keep_alive<0, 1>())
    ;
}

template <typename T>
void addAllRangeMethods(RangeClasses<T> &cls) {
    addCommonIteratorMethods(cls.iterator);
    addCommonIteratorMethods(cls.range);

    cls.iterator
    .def_property_readonly_static("_self_proxy", [](pybind11::object &) -> Proxy<RawIterator<T>> {
        return makeIteratorProxy<T>();
    })
    ;

    cls.range
    .def_property_readonly_static("_self_proxy", [](pybind11::object &) -> Proxy<RawRange<T>> {
        return makeRangeProxy<T>();
    })
    ;
}

#endif /* ranges_py_hpp */
