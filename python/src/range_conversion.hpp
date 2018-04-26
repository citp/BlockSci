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

struct ForcedBool {
    bool val;
};

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
template <> struct type_tag<ForcedBool> { using type = numpy_tag; };
template <> struct type_tag<std::chrono::system_clock::duration> { using type = numpy_tag; };
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
constexpr std::true_type is_range() { return {}; }

template<typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
constexpr std::false_type is_range() { return {}; }

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

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
struct NumpyConverter<blocksci::Address> {
    blocksci::AnyScript operator()(const blocksci::Address &address);
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

template<>
struct NumpyConverter<uint16_t> {
    int64_t operator()(uint16_t val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct NumpyConverter<int16_t> {
    int64_t operator()(int16_t val) {
        return static_cast<int64_t>(val);
    }
};


template<>
struct NumpyConverter<int> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct NumpyConverter<unsigned int> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct NumpyConverter<uint64_t> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

constexpr ranges::category getBlockSciCategory(ranges::category cat) {
    if ((cat & ranges::category::random_access) == ranges::category::random_access) {
        return ranges::category::random_access;
    } else {
        return ranges::category::input;
    }
}

template <typename T>
auto convertRangeToPythonImpl(T && t) {
    auto converted = [&]() {
        using value_type = ranges::range_value_type_t<T>;
        using Converter = NumpyConverter<value_type>;
        if constexpr (is_tagged<value_type>::value) {
            return t;
        } else {
            return t | ranges::view::transform(Converter{});
        }
    }();

    using value_type = ranges::range_value_type_t<decltype(converted)>;
    using range_type = typename type_tag<value_type>::type;

    if constexpr (std::is_same_v<range_type, py_tag>) {
        pybind11::list list;
        RANGES_FOR(auto && a, converted) {
            list.append(std::forward<decltype(a)>(a));
        }
        return list;
    } else if constexpr (std::is_same_v<range_type, numpy_tag>) {
        auto ret = converted | ranges::to_vector;
        return pybind11::array{numpy_dtype<value_type>::value(), ret.size(), ret.data()};
    } else if constexpr (std::is_same_v<range_type, blocksci_tag>) {
        return ranges::any_view<value_type, getBlockSciCategory(ranges::get_categories<T>())>{converted};
    }
}

template <typename T>
auto convertRangeToPython(T && t) {
    if constexpr (is_range<T>()) {
        using value_type = ranges::range_value_type_t<T>;
        // Flatten nested ranges if the nested type is not tagged
        if constexpr (!is_tagged<value_type>::value && is_range<value_type>().value) {
            return convertRangeToPython(std::forward<T>(t) | ranges::view::join);
        } else {
            return convertRangeToPythonImpl(std::forward<T>(t));
        }
    } else {
        return std::forward<T>(t);
    }
}

#endif /* range_conversion_h */
