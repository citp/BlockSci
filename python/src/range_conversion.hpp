//
//  range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_conversion_h
#define range_conversion_h

#include <blocksci/blocksci_fwd.hpp>

#include <pybind11/numpy.h>

#include <range/v3/range_for.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/join.hpp>

#include <chrono>

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

template <typename T, typename=void>
struct is_tagged : std::false_type {};

template <typename T>
struct is_tagged<T, meta::void_<typename type_tag<T>::type>> : std::true_type {};

template<typename T, CONCEPT_REQUIRES_(ranges::Range<T>())>
std::true_type is_range() { return {}; }

template<typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
std::false_type is_range() { return {}; }

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
    std::array<char, 64> operator()(const blocksci::uint256 &val);
};

template<>
struct NumpyConverter<bool> {
    ForcedBool operator()(const bool &val) {
        return {val};
    }
};

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
    return flattenIfNestedRangeImpl<T>(std::forward<T>(t), meta::and_<meta::not_<is_tagged<nested>>, decltype(is_range<nested>())>{});
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

#endif /* range_conversion_h */
