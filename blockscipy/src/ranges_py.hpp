//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "python_fwd.hpp"
#include "blocksci_range.hpp"
#include "proxy.hpp"
#include "generic_sequence.hpp"
#include "proxy_create.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>


template <typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
auto pythonAllType(T && t) {
    return std::forward<T>(t);
}

template <typename T, CONCEPT_REQUIRES_(ranges::Range<T>())>
auto pythonAllType(T && t) {
    pybind11::list list;
    RANGES_FOR(auto && a, std::forward<T>(t)) {
        list.append(pythonAllType(std::forward<decltype(a)>(a)));
    }
    return list;
}


template<typename Class>
auto addCommonIteratorMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__iter__", [](R &range) { 
        return pybind11::make_iterator(range.begin(), range.end()); 
    }, pybind11::keep_alive<0, 1>())
    .def("to_list", [](R & range) { 
        return pythonAllType(range.rng);
    }, "Returns a list of all of the objects in the range")
    ;
}

template<typename Class>
auto addCommonRangeMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__bool__", [](R &range) {
        return !ranges::empty(range.rng);
        
    })
    .def("__len__", [](R &range) {
        return range.rng.size();
    })
    ;
}

template <typename T>
void addAllRangeMethods(RangeClasses<T> &cls) {
    addCommonIteratorMethods(cls.iterator);
    addCommonIteratorMethods(cls.range);
    addCommonRangeMethods(cls.range);

    cls.iterator
    .def_property_readonly_static("self_proxy", [](pybind11::object &) -> Proxy<RawIterator<T>> {
        return makeIteratorProxy<T>();
    })
    ;

    cls.range
    .def_property_readonly_static("self_proxy", [](pybind11::object &) -> Proxy<RawRange<T>> {
        return makeRangeProxy<T>();
    })
    ;
}

#endif /* ranges_py_hpp */
