//
//  ranges_common_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_common_py_hpp
#define ranges_common_py_hpp

#include "blocksci_range.hpp"
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
    using Range = typename Class::type;
    cl
    .def("__iter__", [](Range &range) { 
        return pybind11::make_iterator(range.begin(), range.end()); 
    }, pybind11::keep_alive<0, 1>())
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;

    if constexpr (ranges::ForwardRange<Range>()) {
        cl
        .def("__bool__", [](Range &range) { return !ranges::empty(range); })
        ;
    }

    if constexpr (ranges::BidirectionalRange<Range>()) {
        cl
        .def("__len__", [](Range &range) { return range.size(); })
        .def("__getitem__", [](Range &range, int64_t posIndex) {
            auto chainSize = static_cast<int64_t>(range.size());
            if (posIndex < 0) {
                posIndex += chainSize;
            }
            if (posIndex < 0 || posIndex >= chainSize) {
                throw pybind11::index_error();
            }
            return range[posIndex];
        }, pybind11::arg("index"))

        .def("__getitem__", [](Range &range, pybind11::slice slice) {
            size_t start, stop, step, slicelength;
            const auto &constRange = range;
            auto chainSize = ranges::size(constRange);
            if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();
            
            auto subset =  range[{static_cast<ranges::range_difference_type_t<Range>>(start), static_cast<ranges::range_difference_type_t<Range>>(stop)}];
            auto strided = subset | ranges::view::stride(step);
            return convertRangeToPython(strided);
        }, pybind11::arg("slice"))
        ;
    }
}

#endif /* ranges_common_py_hpp */
