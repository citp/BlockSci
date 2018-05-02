//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "python_fwd.hpp"
#include "range_conversion.hpp"
#include "blocksci_range.hpp"

#include <pybind11/functional.h>

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



template<typename Class>
auto addRangeMethods(Class &cl) {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;

    cl
    .def("__iter__", [](Range &range) { 
        return pybind11::make_iterator(range.begin(), range.end()); 
    }, pybind11::keep_alive<0, 1>())
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")

    // .def("where", [](Range &range, std::function<bool(const value_type &)> query) {
    //     return convertRangeToPython(range | ranges::view::filter(query));
    // }, "Return an iterator over all the items in the sequence matching the given query")
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
        }, pybind11::arg("index"))
        ;
    }

    if constexpr (ranges::BidirectionalRange<Range>()) {
        cl.def("__getitem__", [](Range &range, pybind11::slice slice) {
            size_t start, stop, step, slicelength;
            auto chainSize = ranges::distance(range);
            if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();
            
            auto subset =  ranges::view::slice(range,
                                               static_cast<ranges::range_difference_type_t<Range>>(start),
                                               static_cast<ranges::range_difference_type_t<Range>>(stop));
            return convertRangeToPython(subset | ranges::view::stride(step));
        }, pybind11::arg("slice"));
    }
    
    

    if constexpr(is_optional<value_type>{}) {
        using WrappedType = typename value_type::value_type;

        std::stringstream ss;
        ss << "\n\n:type: :class:`" << PythonTypeName<ranges::any_view<WrappedType, getBlockSciCategory(ranges::get_categories<Range>())>>::name() << "`";
        
        std::stringstream ss2;
        ss2 << "\n\n:type: :class:` numpy.ndarray[bool]`";
        cl
        .def_property_readonly("has_value", [](Range &range) {
            return convertRangeToPython(range | ranges::view::transform([](auto && val) { return val.has_value(); }));
        }, strdup((std::string("Return a array of bools denoting whether a item in the sequence has a value or is none") + ss2.str()).c_str()))
        .def_property_readonly("with_value", [](Range &range) {
            return convertRangeToPython(range | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
            | ranges::view::transform([](const auto &optional) { return *optional; }));
        }, strdup((std::string("Returns a sequence containing only non-None items in the sequence") + ss.str()).c_str()))
        .def("with_default_value", [](Range &range, const WrappedType &defVal) {
            return convertRangeToPython(range | ranges::view::transform([=](const auto &optional) {
                if (optional) {
                    return *optional;
                } else {
                    return defVal;
                }
            }));
        }, pybind11::arg("default_value"), strdup((std::string("Replace all none values in the sequence with the provided default value and return the resulting sequence") + ss.str()).c_str()))
        ;
    }
    return cl;
}

template <typename T>
void addRangeMethods(RangeClasses<T> &cls) {
    addRangeMethods(cls.iterator);
    addRangeMethods(cls.range);
    addRangeMethods(cls.optionalIterator);
    addRangeMethods(cls.optionalRange);
}

#endif /* ranges_py_hpp */
