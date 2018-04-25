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

template<typename Range, typename... Extra, CONCEPT_REQUIRES_(ranges::RandomAccessRange<Range>() && ranges::SizedRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    pybind11::class_<Range> cl(m, name.c_str(), extra...);
    cl
    .def("__len__", [](Range &chain) { return chain.size(); })
    .def("__bool__", [](Range &range) { return !ranges::empty(range); })
    .def("__iter__", [](Range &chain) { return pybind11::make_iterator(chain.begin(), chain.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &chain, int64_t posIndex) {
        auto chainSize = static_cast<int64_t>(chain.size());
        if (posIndex < 0) {
            posIndex += chainSize;
        }
        if (posIndex < 0 || posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        return chain[posIndex];
    })
    .def("__getitem__", [](Range &chain, pybind11::slice slice) -> ranges::any_view<decltype(chain[0]), ranges::category::random_access> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        
        auto subset =  ranges::view::slice(chain,
                                           static_cast<ranges::range_difference_type_t<Range>>(start),
                                           static_cast<ranges::range_difference_type_t<Range>>(stop));
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    .def_property_readonly("all", [](Range & range) {
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
    return cl;
}

template<typename Range, typename... Extra, CONCEPT_REQUIRES_(ranges::RandomAccessRange<Range>() && !ranges::SizedRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    pybind11::class_<Range> cl(m, name.c_str(), extra...);
    cl
    .def("__len__", [](Range &chain) { return ranges::distance(chain); })
    .def("__bool__", [](Range &range) { return !ranges::empty(range); })
    .def("__iter__", [](Range &chain) { return pybind11::make_iterator(chain.begin(), chain.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &chain, int64_t posIndex) {
        auto chainSize = ranges::distance(chain);
        if (posIndex < 0) {
            posIndex += chainSize;
        }
        if (posIndex < 0 || posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        return chain[posIndex];
    })
    .def("__getitem__", [](Range &chain, pybind11::slice slice) -> ranges::any_view<decltype(chain.front()), ranges::category::random_access> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(ranges::distance(chain), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        auto subset =  ranges::view::slice(chain,
                                           static_cast<ranges::range_difference_type_t<Range>>(start),
                                           static_cast<ranges::range_difference_type_t<Range>>(stop));
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
    return cl;
}

template<typename Range, typename... Extra, CONCEPT_REQUIRES_(ranges::BidirectionalRange<Range>() && !ranges::RandomAccessRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    pybind11::class_<Range> cl(m, name.c_str(), extra...);
    cl
    .def("__len__", [](Range &range) { return ranges::distance(range); })
    .def("__bool__", [](Range &range) { return !ranges::empty(range); })
    .def("__iter__", [](Range &range) { return pybind11::make_iterator(range.begin(), range.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &range, int64_t posIndex) {
        auto chainSize = ranges::distance(range);
        if (posIndex < 0) {
            posIndex += chainSize;
        }
        if (posIndex < 0 || posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        return range[posIndex];
    })
    .def("__getitem__", [](Range &range, pybind11::slice slice) -> ranges::any_view<decltype(*range.begin())> {
        size_t start, stop, step, slicelength;
        auto chainSize = ranges::distance(range);
        if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        auto subset =  ranges::view::slice(range, start, stop);
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
    return cl;
}

template<typename Range, typename... Extra, CONCEPT_REQUIRES_(ranges::InputRange<Range>() && !ranges::ForwardRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    pybind11::class_<Range> cl(m, name.c_str(), extra...);
    cl
    .def("__iter__", [](Range &range) { return pybind11::make_iterator(range.begin(), range.end()); },
         pybind11::keep_alive<0, 1>())
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
    return cl;
}

template<typename Range, typename... Extra>
auto addOptionalRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    using WrappedType = decltype(*std::declval<ranges::range_value_type_t<Range>>());
    auto cl = addRangeClass<Range>(m, name, extra...);
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
    return cl;
}

#endif /* ranges_py_hpp */
