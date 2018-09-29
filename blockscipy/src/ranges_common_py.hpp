//
//  ranges_common_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_common_py_hpp
#define ranges_common_py_hpp

#include "blocksci_range.hpp"
#include "range_utils.hpp"
#include "range_conversion.hpp"

#include <range/v3/range_for.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>

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
auto addGenericRangeMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__iter__", [](R &range) { 
        return pybind11::make_iterator(range.begin(), range.end()); 
    }, pybind11::keep_alive<0, 1>())
    .def_property_readonly("all", [](R & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;

    if constexpr (ranges::ForwardRange<R>()) {
        cl
        .def("__bool__", [](R &range) { return !ranges::empty(range); })
        ;
    }

    if constexpr (ranges::BidirectionalRange<R>()) {
        cl
        .def("__len__", [](R &range) { return range.size(); })
        .def("__getitem__", [](R &range, int64_t posIndex) {
            auto chainSize = static_cast<int64_t>(range.size());
            if (posIndex < 0) {
                posIndex += chainSize;
            }
            if (posIndex < 0 || posIndex >= chainSize) {
                throw pybind11::index_error();
            }
            return range[posIndex];
        }, pybind11::arg("index"))

        .def("__getitem__", [](R &range, pybind11::slice slice) -> Range<ranges::range_value_type_t<R>> {
            size_t start, stop, step, slicelength;
            const auto &constRange = range;
            auto chainSize = ranges::size(constRange);
            if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();
            
            auto subset =  range[{static_cast<ranges::range_size_type_t<R>>(start), static_cast<ranges::range_size_type_t<R>>(stop)}];
            return subset | ranges::view::stride(step);
        }, pybind11::arg("slice"))
        ;
    }
}

#endif /* ranges_common_py_hpp */
