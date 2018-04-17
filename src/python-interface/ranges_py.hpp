//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "variant_py.hpp"
#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/range_for.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>
#include <pybind11/stl_bind.h>

#include <chrono>
#include <iostream>

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
struct NumpyConverter {
    auto operator()(const T &val) {
        return val;
    }
};

template<>
struct NumpyConverter<std::chrono::system_clock::time_point> {
    auto operator()(const std::chrono::system_clock::time_point &val) {
        return val.time_since_epoch();
    }
};

template<>
struct NumpyConverter<blocksci::uint256> {
    auto operator()(const blocksci::uint256 &val) {
        auto hexStr = val.GetHex();
        std::array<char, 64> ret;
        std::copy_n(hexStr.begin(), 64, ret.begin());
        return ret;
    }
};

template<>
struct NumpyConverter<bool> {
    ForcedBool operator()(const bool &val) {
        return {val};
    }
};

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

struct unknown_tag {};
struct blocksci_tag {};
struct py_tag {};
struct numpy_tag {};

template <typename T>
struct type_tag;

template <typename T> struct type_tag<ranges::optional<T>> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Blockchain> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Block> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Transaction> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Input> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Output> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::AnyScript> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::EquivAddress> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::Pubkey> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::PubkeyHash> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::MultisigPubkey> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::WitnessPubkeyHash> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::ScriptHash> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::WitnessScriptHash> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::Multisig> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::OpReturn> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::script::Nonstandard> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::ClusterManager> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Cluster> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::TaggedCluster> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::TaggedAddress> { using type = blocksci_tag; };

template <> struct type_tag<int64_t> { using type = numpy_tag; };
template <> struct type_tag<bool> { using type = numpy_tag; };
template <> struct type_tag<std::chrono::system_clock::time_point> { using type = numpy_tag; };
template <> struct type_tag<blocksci::uint256> { using type = numpy_tag; };
template <> struct type_tag<blocksci::uint160> { using type = numpy_tag; };
template <> struct type_tag<pybind11::bytes> { using type = numpy_tag; };

template <> struct type_tag<pybind11::list> { using type = py_tag; };
template <> struct type_tag<std::string> { using type = py_tag; };
template <> struct type_tag<blocksci::AddressType::Enum> { using type = py_tag; };

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

template <typename T, typename=void>
struct is_tagged : std::false_type {};

template <typename T>
struct is_tagged<T, meta::void_<typename type_tag<T>::type>> : std::true_type {};

template <typename T, typename=void>
struct is_blocksci : std::false_type {};

template <typename T>
struct is_blocksci<T, std::enable_if_t<std::is_same<typename type_tag<T>::type, blocksci_tag>::value, void>> : std::true_type {};

template<typename T, CONCEPT_REQUIRES_(ranges::Range<T>())>
std::true_type is_range() { return {}; }

template<typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
std::false_type is_range() { return {}; }

template <typename T>
struct make_optional { using type = ranges::optional<T>; };

template <typename T>
struct make_optional<ranges::optional<T>> { using type = ranges::optional<T>; };

template <typename T>
using make_optional_t = typename make_optional<T>::type;

template <typename T, class F, std::size_t ... Is, class Trait, typename... Args>
auto applyMethodToRangeImpl(T && t, F func, std::index_sequence<Is...>, Trait, std::true_type, const Args &... args) {
    return std::forward<T>(t) | ranges::view::transform([=](auto && item) -> make_optional_t<decltype(func(*item, args...))> {
        if (item) {
            return func(*item, args...);
        } else {
            return ranges::nullopt;
        }
    });
}

template <typename T, class F, std::size_t ... Is, class Trait, typename... Args>
auto applyMethodToRangeImpl(T && t, F func, std::index_sequence<Is...>, Trait, std::false_type, const Args &... args) {
    return std::forward<T>(t) | ranges::view::transform([=](auto && item) {
        return func(item, args...);
    });
}

template <typename T, typename F, typename... Args, CONCEPT_REQUIRES_(ranges::Range<T>())>
auto applyMethodToRange(T && t, F func, const Args &... args) {
    using traits = function_traits<F>;
    return applyMethodToRangeImpl<T>(std::forward<T>(t), func, std::make_index_sequence<traits::arity>{}, traits{}, is_optional<ranges::range_value_type_t<T>>{}, args...);
}

template <typename T>
auto flattenIfNestedRangeImpl(T && t, std::true_type) {
    return std::forward<T>(t) | ranges::view::join;
}

template <typename T>
auto flattenIfNestedRangeImpl(T && t, std::false_type) {
    return std::forward<T>(t);
}

template <typename T>
auto flattenIfNestedRange(T && t) {
    using nested = ranges::range_value_type_t<T>;
    return flattenIfNestedRangeImpl<T>(std::forward<T>(t), and_<meta::not_<is_tagged<nested>>, decltype(is_range<nested>())>{});
}

template <typename T>
auto flattenIfOptionalRangeImpl(T && t, std::true_type) {
    return std::forward<T>(t) | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
    | ranges::view::transform([](const auto &optional) { return *optional; });
}

template <typename T>
auto flattenIfOptionalRangeImpl(T && t, std::false_type) {
    return std::forward<T>(t);
}

template <typename T>
auto flattenIfOptionalRange(T && t) {
    return flattenIfOptionalRangeImpl<T>(std::forward<T>(t), is_optional<ranges::range_value_type_t<T>>{});
}

template <typename T, CONCEPT_REQUIRES_(ranges::RandomAccessRange<T>())>
auto convertRangeToPythonImpl(T && t, blocksci_tag) -> ranges::any_view<ranges::range_value_type_t<T>, ranges::category::random_access> {
    return std::forward<T>(t);
}

template <typename T, CONCEPT_REQUIRES_(!ranges::RandomAccessRange<T>())>
auto convertRangeToPythonImpl(T && t, blocksci_tag) -> ranges::any_view<ranges::range_value_type_t<T>> {
    return std::forward<T>(t);
}

template <typename T>
auto convertRangeToPythonImpl(T && t, numpy_tag) {
    using RangeType = ranges::range_value_type_t<T>;
    using Converter = NumpyConverter<RangeType>;
    using vector_value_type = decltype(Converter{}(std::declval<RangeType>()));
    auto ret = std::forward<T>(t) | ranges::view::transform([](auto &&item) { return Converter{}(std::forward<decltype(item)>(item)); }) | ranges::to_vector;
    return pybind11::array{numpy_dtype<vector_value_type>::value(), ret.size(), ret.data()};
}

template <typename T>
auto convertRangeToPythonImpl(T && t, py_tag) {
    pybind11::list list;
    RANGES_FOR(auto && a, std::forward<T>(t)) {
        list.append(std::forward<decltype(a)>(a));
    }
    return list;
}

template <typename T, CONCEPT_REQUIRES_(ranges::Range<T>())>
auto convertRangeToPython(T && t) {
    auto && flattened = flattenIfNestedRange(t);
    return convertRangeToPythonImpl(std::forward<decltype(flattened)>(flattened), typename type_tag<ranges::range_value_type_t<decltype(flattened)>>::type{});
}

template <typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
auto convertRangeToPython(T && t) {
    return std::forward<T>(t);
}

template <typename T, typename F, std::size_t ... Is, class Traits>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, Traits) {
    return [f](T &view, const std::tuple_element_t<Is, typename Traits::arg_tuple> &... args) {
        return convertRangeToPython(applyMethodToRange(view, f, args...));
    };
}

template <typename T, typename F>
auto applyMethodsToRange(F f) {
    using traits = function_traits<F>;
    return applyMethodsToRangeImpl<T>(f, std::make_index_sequence<traits::arity>{}, traits{});
}

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToSelfImpl(F f, std::index_sequence<Is...>, T) {
    return [f](Ty &tx, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        return f(tx, args...);
    };
}

template <typename Ty,typename F>
auto applyMethodsToSelf(F f) {
    using traits = function_traits<F>;
    return applyMethodsToSelfImpl<Ty>(f, std::make_index_sequence<traits::arity>{}, traits{});
}

template <typename T, typename F, std::size_t ... Is, class Traits>
auto applyRangeMethodsToRangeImpl(F func, std::index_sequence<Is...>, Traits) {
    return [func](T &view, const std::tuple_element_t<Is, typename Traits::arg_tuple> &... args) {
        return convertRangeToPython(func(flattenIfOptionalRange(view), args...));
    };
}

template <typename T, typename F>
auto applyRangeMethodsToRangeImpl1(F func) {
    using traits = function_traits<F>;
    return applyRangeMethodsToRangeImpl<T>(func, std::make_index_sequence<traits::arity>{}, traits{});
}

template <template <typename, typename, typename> class F, typename Class>
auto applyRangeMethodsToRange(Class &cl) {
    using Range = typename Class::type;
    using flattened_type = decltype(flattenIfOptionalRange(std::declval<Range>()));
    auto handler = [](auto func) {
        return applyRangeMethodsToRangeImpl1<flattened_type>(func);
    };
    F<flattened_type, decltype(cl), decltype(handler)>{}(cl, handler);
}

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
