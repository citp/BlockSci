//
//  range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_conversion_h
#define range_conversion_h

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/chain/range_util.hpp>

#include <pybind11/numpy.h>

#include <range/v3/range_for.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/join.hpp>

#include <chrono>

struct NumpyBool {
    bool val;
};

struct NumpyDatetime {
    int64_t time;
};

struct blocksci_tag {};
struct py_tag {};
struct numpy_tag {};

template <typename T, typename = void>
struct type_tag;

template <typename T, typename=void>
struct is_tagged : std::false_type {};

template <typename T>
struct is_tagged<T, meta::void_<typename type_tag<T>::type>> : std::true_type {};

template <> struct type_tag<blocksci::Blockchain> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Block> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Transaction> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Input> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Output> { using type = blocksci_tag; };
template <> struct type_tag<blocksci::Address> { using type = blocksci_tag; };
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

template <> struct type_tag<int16_t> { using type = numpy_tag; };
template <> struct type_tag<uint16_t> { using type = numpy_tag; };
template <> struct type_tag<int32_t> { using type = numpy_tag; };
template <> struct type_tag<uint32_t> { using type = numpy_tag; };
template <> struct type_tag<int64_t> { using type = numpy_tag; };
template <> struct type_tag<uint64_t> { using type = numpy_tag; };
template <> struct type_tag<double> { using type = numpy_tag; };
template <> struct type_tag<bool> { using type = numpy_tag; };
template <> struct type_tag<std::chrono::system_clock::time_point> { using type = numpy_tag; };
template <> struct type_tag<blocksci::uint256> { using type = numpy_tag; };
template <> struct type_tag<blocksci::uint160> { using type = numpy_tag; };

template <> struct type_tag<pybind11::bytes> { using type = py_tag; };
template <> struct type_tag<pybind11::list> { using type = py_tag; };
template <> struct type_tag<std::string> { using type = py_tag; };
template <> struct type_tag<blocksci::AddressType::Enum> { using type = py_tag; };

template <typename A, typename B> struct type_tag<std::pair<A, B>> { using type = py_tag; };

template <typename T>
struct type_tag<ranges::optional<T>, std::enable_if_t<is_tagged<T>::value>> { using type = blocksci_tag; };

struct BlockSciTypeConverter {
    template <typename T>
    auto operator()(const T &val) {
        return val;
    }

    blocksci::AnyScript operator()(const blocksci::Address &address);

    int64_t operator()(uint16_t val) {
        return static_cast<int64_t>(val);
    }

    int64_t operator()(int16_t val) {
        return static_cast<int64_t>(val);
    }

    int64_t operator()(uint32_t val) {
        return static_cast<int64_t>(val);
    }

    int64_t operator()(int32_t val) {
        return static_cast<int64_t>(val);
    }

    // Potential overflow
    int64_t operator()(uint64_t val) {
        return static_cast<int64_t>(val);
    }

    int64_t operator()(int64_t val) {
        return static_cast<int64_t>(val);
    }
    
    double operator()(double val) {
        return static_cast<double>(val);
    }

    template <typename A, typename B>
    auto operator()(const std::pair<A, B> &val) -> std::pair<decltype(this->operator()(val.first)), decltype(this->operator()(val.second))> {
        return {this->operator()(val.first), this->operator()(val.second)};
    }

    template <typename T>
    auto operator()(const ranges::optional<T> &val) -> ranges::optional<decltype(this->operator()(*val))> {
        if (val) {
            return this->operator()(*val);
        } else {
            return ranges::nullopt;
        }
    }
};

template <typename T>
struct NumpyConverter {
    using type = T;
    type operator()(const T &val) {
        return val;
    }
};

template<>
struct NumpyConverter<std::chrono::system_clock::time_point> {
    using type = NumpyDatetime;
    type operator()(const std::chrono::system_clock::time_point &val);
};

template<>
struct NumpyConverter<blocksci::uint256> {
    using type = std::array<char, 64>;
    type operator()(const blocksci::uint256 &val);
};

template<>
struct NumpyConverter<blocksci::uint160> {
    using type = std::array<char, 40>;
    type operator()(const blocksci::uint160 &val);
};

template<>
struct NumpyConverter<bool> {
    using type = NumpyBool;
    type operator()(const bool &val) {
        return {val};
    }
};

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

// If type is already optional, do nothing. Otherwise make it optional
template <typename T>
struct make_optional { using type = ranges::optional<T>; };

template <typename T>
struct make_optional<ranges::optional<T>> { using type = ranges::optional<T>; };

template <typename T>
using make_optional_t = typename make_optional<T>::type;

template<class T> struct dependent_false : std::false_type {};

constexpr ranges::category getBlockSciCategory(ranges::category cat) {
    constexpr auto randomSized = ranges::category::random_access | ranges::category::sized;
    if ((cat & randomSized) == randomSized) {
        return randomSized;
    } else {
        return ranges::category::input;
    }
}

template <ranges::category range_cat, typename T, typename tag = typename type_tag<T>::type>
struct ConvertedTagImpl;

template <ranges::category range_cat, typename T>
struct ConvertedTagImpl<range_cat, T, py_tag> {
    using type = pybind11::list;
};

template <ranges::category range_cat, typename T>
struct ConvertedTagImpl<range_cat, T, numpy_tag> {
    using type = pybind11::array_t<typename NumpyConverter<T>::type>;
};

template <ranges::category range_cat, typename T>
struct ConvertedTagImpl<range_cat, T, blocksci_tag> {
    using type = ranges::any_view<decltype(BlockSciTypeConverter{}(std::declval<T>())), range_cat>;
};

template <typename T>
using converted_range_impl_t = typename ConvertedTagImpl<getBlockSciCategory(ranges::get_categories<T>()), ranges::range_value_type_t<T>>::type;

template <typename T>
constexpr bool isNonTaggedRange() {
    return !is_tagged<T>::value && ranges::Range<T>();
}

template <typename T>
constexpr bool isNestedOptionalRange() {
    using value_type = ranges::range_value_type_t<T>;
    if constexpr (is_optional<value_type>::value) {
        if constexpr (isNonTaggedRange<typename value_type::value_type>()) {
            return true;
        }
    }
    return false;
}

template <typename T>
constexpr bool isNestedRange() {
    using value_type = ranges::range_value_type_t<T>;
    if constexpr (isNonTaggedRange<value_type>()) {
            return true;
        }
    return false;
}

enum class RangeTypeTag {
    Normal, Nested, NestedOptional
};

template <typename T>
constexpr RangeTypeTag getRangeTypeTag() {
    if (isNestedOptionalRange<T>()) {
        return RangeTypeTag::NestedOptional;
    } else if (isNestedRange<T>()) {
        return RangeTypeTag::Nested;
    } else {
        return RangeTypeTag::Normal;
    }
}

template <typename T, RangeTypeTag tag>
struct ConvertedRangeTagImpl;

template <typename T>
struct ConvertedRangeTagImpl<T, RangeTypeTag::Normal> {
    using type = converted_range_impl_t<T>;
};

template <typename T>
struct ConvertedRangeTagImpl<T, RangeTypeTag::Nested> {
    using type = converted_range_impl_t<decltype(std::declval<T>() | ranges::view::join)>;
};

template <typename T>
struct ConvertedRangeTagImpl<T, RangeTypeTag::NestedOptional> {
    using type = converted_range_impl_t<decltype(std::declval<T>() | blocksci::flatMapOptionals() | ranges::view::join)>;
};

template <typename T>
using converted_range_t = typename ConvertedRangeTagImpl<T, getRangeTypeTag<T>()>::type;

template <typename T>
struct is_any_view : std::false_type {};

template <typename T>
struct is_any_view<ranges::any_view<T, ranges::category::random_access | ranges::category::sized>> : std::true_type {};

template <typename T>
struct is_any_view<ranges::any_view<T>> : std::true_type {};

template <typename T>
constexpr bool isNonTaggedNonAnyViewRange() {
    return isNonTaggedRange<T>() && !is_any_view<T>::value;
}

template <typename T>
constexpr bool isNonTaggedOptionalNonAnyViewRange() {
    if constexpr (is_optional<T>::value) {
        return isNonTaggedNonAnyViewRange<typename T::value_type>();
    }
    return false;
}

template <typename T>
auto convertToGeneric(T && t) {
    using value_type = ranges::range_value_type_t<T>;
    constexpr auto range_cat = getBlockSciCategory(ranges::get_categories<T>());
    if constexpr (isNonTaggedNonAnyViewRange<value_type>()) {
         return convertToGeneric(std::forward<T>(t) | ranges::view::transform([](auto && subrange) {
            return convertToGeneric(std::forward<decltype(subrange)>(subrange));
         }));
    } else if constexpr (isNonTaggedOptionalNonAnyViewRange<value_type>()) {
        return convertToGeneric(std::forward<T>(t) | ranges::view::transform([](auto && subrange) -> make_optional_t<decltype(convertToGeneric(*subrange))> {
            if (subrange) {
                return convertToGeneric(*subrange);
            } else {
                return ranges::nullopt;
            }
         }));
    } else {
        return ranges::any_view<value_type, range_cat>{std::forward<T>(t)};
    }
}

template <typename T>
converted_range_t<T> convertAnyRangeToPython(T && t);

template <typename T>
converted_range_t<T> convertRangeToPython(T && t) {
    // Flatten nested ranges if the nested type is not tagged
    if constexpr (isNestedRange<T>()) {
        return convertRangeToPython(std::forward<T>(t) | ranges::view::join);
    } else if constexpr (isNestedOptionalRange<T>()) {
        // Flatten optional nested ranges
        return convertRangeToPython(std::forward<T>(t) | blocksci::flatMapOptionals() | ranges::view::join);
    } else {
        return convertAnyRangeToPython(convertToGeneric(std::forward<T>(t)));
    }
}

#endif /* range_conversion_h */