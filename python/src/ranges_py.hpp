//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "range_conversion.hpp"

#include <range/v3/range_for.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/transform.hpp>

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

template<typename Range, typename... Extra>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    using value_type = ranges::range_value_type_t<Range>;
    pybind11::class_<Range> cl(m, name.c_str(), extra...);

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
        .def("__len__", [](Range &range) { return ranges::distance(range); })
        .def("__getitem__", [](Range &range, int64_t posIndex) {
            auto chainSize = static_cast<int64_t>(ranges::distance(range));
            if (posIndex < 0) {
                posIndex += chainSize;
            }
            if (posIndex < 0 || posIndex >= chainSize) {
                throw pybind11::index_error();
            }
            return range[posIndex];
        })
        ;
    }

    if constexpr (ranges::BidirectionalRange<Range>()) {
        cl.def("__getitem__", [](Range &range, pybind11::slice slice) -> ranges::any_view<value_type, getBlockSciCategory(ranges::get_categories<Range>())> {
            size_t start, stop, step, slicelength;
            auto chainSize = ranges::distance(range);
            if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();
            
            auto subset =  ranges::view::slice(range,
                                               static_cast<ranges::range_difference_type_t<Range>>(start),
                                               static_cast<ranges::range_difference_type_t<Range>>(stop));
            return subset | ranges::view::stride(step);
        });
    }
    
    if constexpr(is_optional<value_type>{}) {
        using WrappedType = decltype(*std::declval<ranges::range_value_type_t<Range>>());
        cl
        .def_property_readonly("has_value", [](Range &range) {
            return convertRangeToPython(range | ranges::view::transform([](auto && val) { return val.has_value(); }));
        })
        .def_property_readonly("with_value", [](Range &range) {
            return convertRangeToPython(range | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
            | ranges::view::transform([](const auto &optional) { return *optional; }));
        })
        .def("with_default_value", [](Range &range, const WrappedType &defVal) {
            return convertRangeToPython(range | ranges::view::transform([=](const auto &optional) {
                if (optional) {
                    return *optional;
                } else {
                    return defVal;
                }
            }));
        })
        ;
    }
    return cl;
}

#endif /* ranges_py_hpp */
