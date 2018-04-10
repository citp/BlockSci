//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range_for.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>

#include <iostream>


//template <typename T>
//struct NativeBlockSciListType : std::false_type;
//struct NativeBlockSciListType<blocksci::Block> : std::true_type {};
//struct NativeBlockSciListType<blocksci::Transaction> : std::true_type {};
//
//struct LazyRange {};
//struct NumpyRange {};
//struct PythonList {};
//
//template <typename T>
//struct BlockSciPythonRangeType;
//
//template <typename Duration>
//struct BlockSciPythonRangeType<std::chrono::time_point<std::chrono::system_clock, Duration>> {
//    using type = py::array
//}

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

struct ForcedBool {
    bool val;
};

template <typename T>
struct numpy_dtype {
    static pybind11::dtype value() {
        return pybind11::dtype::of<T>();
    };
};

template <>
struct numpy_dtype<std::chrono::system_clock::duration> {
    static pybind11::dtype value() {
        return pybind11::dtype{"datetime64[ns]"};
    }
};

template <>
struct numpy_dtype<std::array<char, 64>> {
    static pybind11::dtype value() {
        return pybind11::dtype{"S64"};
    }
};

template <>
struct numpy_dtype<pybind11::bytes> {
    static pybind11::dtype value() {
        return pybind11::dtype{"bytes"};
    }
};

template <>
struct numpy_dtype<ForcedBool> {
    static pybind11::dtype value() {
        return pybind11::dtype{"bool"};
    }
};


template <typename T>
inline auto toNumpy(T && val) {
    return std::forward<T>(val);
}

inline auto toNumpy(std::chrono::system_clock::time_point && time) {
    return time.time_since_epoch();
}

inline std::array<char, 64> toNumpy(blocksci::uint256 && s) {
    auto hexStr = s.GetHex();
    std::array<char, 64> ret;
    std::copy_n(hexStr.begin(), 64, ret.begin());
    return ret;
}

inline ForcedBool toNumpy(bool && s) {
    return {s};
}

template <typename T>
struct is_blocksci_type : std::false_type {};

template <>
struct is_blocksci_type<blocksci::Blockchain> : std::true_type {};

template <>
struct is_blocksci_type<blocksci::Block> : std::true_type {};

template <>
struct is_blocksci_type<blocksci::Transaction> : std::true_type {};

template <>
struct is_blocksci_type<ranges::optional<blocksci::Transaction>> : std::true_type {};

template <>
struct is_blocksci_type<blocksci::Input> : std::true_type {};

template <>
struct is_blocksci_type<ranges::optional<blocksci::Input>> : std::true_type {};

template <>
struct is_blocksci_type<blocksci::Output> : std::true_type {};

template <>
struct is_blocksci_type<ranges::optional<blocksci::Output>> : std::true_type {};

template <>
struct is_blocksci_type<blocksci::AnyScript::ScriptVariant> : std::true_type {};

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, T, std::false_type, std::false_type) {
    return [&f](Ty &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        using result_type = typename T::result_type;
        using vector_value_type = decltype(toNumpy(std::declval<result_type>()));
        std::vector<vector_value_type> ret;
        RANGES_FOR(auto && tx, view) {
            ret.push_back(toNumpy(f(std::forward<decltype(tx)>(tx), args...)));
        }
        return pybind11::array{numpy_dtype<vector_value_type>::value(), ret.size(), ret.data()};
    };
}

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, T, std::true_type, std::false_type) {
    return [&f](Ty &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) -> ranges::any_view<typename T::result_type, ranges::get_categories<Ty>()> {
        return view | ranges::view::transform([=](auto && item) {
            return f(item, args...);
        });
    };
}

template <typename Range, class F, std::size_t ... Is, class T, typename O>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, T, O, std::true_type) {
    return [&f](Range &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) -> ranges::any_view<ranges::optional<typename T::result_type>, ranges::get_categories<Range>()> {
        return view | ranges::view::transform([=](auto && item) -> ranges::optional<typename T::result_type> {
            if (item) {
                return f(*item, args...);
            } else {
                return ranges::nullopt;
            }
        });
    };
}

//forms_normal_vector<typename traits::result_type>{}
template <typename T, typename F>
auto applyMethodsToRange(F f) {
    using traits = function_traits<F>;
    return applyMethodsToRangeImpl<T>(f, std::make_index_sequence<traits::arity>{}, traits{}, std::integral_constant<bool, is_blocksci_type<typename traits::result_type>::value>{}, is_optional<ranges::range_value_type_t<T>>{});
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
