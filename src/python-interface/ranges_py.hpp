//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include <blocksci/chain/block.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/range_for.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>

#include <iostream>


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
void addNestedRangeMethodsImpl(Class &, std::false_type) {}

template<typename Class>
void addNestedRangeMethodsImpl(Class &cl, std::true_type) {
    using Range = typename Class::type;
    cl
    .def_property_readonly("flatten", [](Range &view) -> ranges::any_view<ranges::range_value_type_t<ranges::range_value_type_t<Range>>>{
        return view | ranges::view::join;
    })
    ;
}

template<typename Class>
void addNestedRangeMethods(Class &cl) {
    using Range = typename Class::type;
    return addNestedRangeMethodsImpl(cl, ranges::Range<ranges::range_value_type_t<Range>>{});
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
        auto chainSize = chain.size();
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
    addNestedRangeMethods(cl);
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
    addNestedRangeMethods(cl);
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
    addNestedRangeMethods(cl);
    return cl;
}

template<typename Range, typename... Extra, CONCEPT_REQUIRES_(ranges::InputRange<Range>() && !ranges::ForwardRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name, const Extra &... extra) {
    pybind11::class_<Range> cl(m, name.c_str(), extra...);
    cl
    .def("__len__", [](Range &range) { return ranges::distance(range); })
    .def("__bool__", [](Range &range) { return ranges::distance(range) > 0; })
    .def_property_readonly("all", [](Range & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
    addNestedRangeMethods(cl);
    return cl;
}

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())>
{};

template <typename ClassType, typename ReturnType, typename First, typename... Args>
struct function_traits<ReturnType(ClassType::*)(First, Args...) const>
// we specialize for pointers to member function
{
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
    static constexpr auto arity = sizeof...(Args);
};

template<typename... Conds>
struct and_
: std::true_type
{ };

template<typename Cond, typename... Conds>
struct and_<Cond, Conds...>
: std::conditional<Cond::value, and_<Conds...>, std::false_type>::type
{ };

template <typename T>
using forms_normal_vector = pybind11::detail::vector_has_data_and_format<std::vector<T>>;

template <typename T>
using is_numeric = pybind11::detail::satisfies_any_of<T, std::is_arithmetic, pybind11::detail::is_complex>;

template <typename T>
using numpy_transformable = and_<is_numeric<T>, forms_normal_vector<T>>;

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, T, std::true_type) {
    return [&f](Ty &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        std::vector<typename T::result_type> ret;
        RANGES_FOR(auto && tx, view) {
            ret.push_back(f(std::forward<decltype(tx)>(tx), args...));
        }
        return pybind11::array_t<typename T::result_type>(ret.size(), ret.data());
    };
}

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, T, std::false_type) {
    return [&f](Ty &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        pybind11::list list;
        RANGES_FOR(auto && tx, view) {
            list.append(f(std::forward<decltype(tx)>(tx), args...));
        }
        return list;
    };
}

template <typename T, typename F>
auto applyMethodsToRange(F f) {
    using traits = function_traits<F>;
    return applyMethodsToRangeImpl<T>(f, std::make_index_sequence<traits::arity>{}, traits{}, numpy_transformable<typename traits::result_type>{});
}


template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToSelfImpl(F f, std::index_sequence<Is...>, T) {
    return [&f](const Ty &tx, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        return f(std::forward<decltype(tx)>(tx), args...);
    };
}

template <typename Ty,typename F>
auto applyMethodsToSelf(F f) {
    using traits = function_traits<F>;
    return applyMethodsToSelfImpl<Ty>(f, std::make_index_sequence<traits::arity>{}, traits{});
}


#endif /* ranges_py_hpp */
