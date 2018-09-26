//
//  range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_conversion_h
#define range_conversion_h

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/chain/input.hpp>

#include <pybind11/numpy.h>

#include <range/v3/view/any_view.hpp>

#include <chrono>

struct NumpyBool {
    bool val;
};

struct NumpyDatetime {
    int64_t time;
};

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

namespace pybind11 { namespace detail {
    template <>
    struct npy_format_descriptor<NumpyDatetime> { 
        static PYBIND11_DESCR name() { return _("datetime64[ns]"); }
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("datetime64[ns]")); }
    };

    template <>
    struct npy_format_descriptor<NumpyBool> { 
        static PYBIND11_DESCR name() { return _("bool"); }
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("bool")); }
    };
}}

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

struct PythonConversionTypeConverter {
    pybind11::list operator()(ranges::any_view<pybind11::bytes> && t);
    pybind11::list operator()(ranges::any_view<pybind11::list> && t);
    pybind11::list operator()(ranges::any_view<std::string> && t);
    pybind11::list operator()(ranges::any_view<blocksci::AddressType::Enum> && t);

    pybind11::list operator()(ranges::any_view<pybind11::bytes, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<pybind11::list, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<std::string, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<blocksci::AddressType::Enum, random_access_sized> && t);

    pybind11::array_t<int64_t> operator()(ranges::any_view<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(ranges::any_view<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(ranges::any_view<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(ranges::any_view<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(ranges::any_view<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(ranges::any_view<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(ranges::any_view<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(ranges::any_view<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(ranges::any_view<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(ranges::any_view<blocksci::uint256> && t);

    pybind11::array_t<int64_t> operator()(ranges::any_view<int64_t, random_access_sized> && t);
    pybind11::array_t<uint64_t> operator()(ranges::any_view<uint64_t, random_access_sized> && t);
    pybind11::array_t<int32_t> operator()(ranges::any_view<int32_t, random_access_sized> && t);
    pybind11::array_t<uint32_t> operator()(ranges::any_view<uint32_t, random_access_sized> && t);
    pybind11::array_t<int16_t> operator()(ranges::any_view<int16_t, random_access_sized> && t);
    pybind11::array_t<uint16_t> operator()(ranges::any_view<uint16_t, random_access_sized> && t);
    pybind11::array_t<NumpyBool> operator()(ranges::any_view<bool, random_access_sized> && t);
    pybind11::array_t<NumpyDatetime> operator()(ranges::any_view<std::chrono::system_clock::time_point, random_access_sized> && t);
    pybind11::array_t<std::array<char, 40>> operator()(ranges::any_view<blocksci::uint160, random_access_sized> && t);
    pybind11::array_t<std::array<char, 64>> operator()(ranges::any_view<blocksci::uint256, random_access_sized> && t);

    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<int16_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint16_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<int32_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint32_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint64_t>> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<blocksci::Address> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<blocksci::Address>> && t);

    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<int16_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint16_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<int32_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint32_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint64_t>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript, random_access_sized> operator()(ranges::any_view<blocksci::Address, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized> operator()(ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized> && t);

    template <typename T>
    ranges::any_view<T> operator()(ranges::any_view<T> && t) {
        return t;
    }

    template <typename T>
    ranges::any_view<T, random_access_sized> operator()(ranges::any_view<T, random_access_sized> && t) {
        return t;
    }
};

enum class RangeTypeTag {
    Normal, Nested, NestedOptional
};

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

template <typename T>
constexpr bool isNonTaggedRange() {
    return !is_tagged<T>::value && ranges::Range<T>();
}

constexpr auto isOptional = [](const auto &optional) { return static_cast<bool>(optional); };
constexpr auto derefOptional = [](const auto &optional) { return *optional; };

struct ConvertedRangeTypeTagConverter {
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::any_view<blocksci::Input, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::any_view<blocksci::Input>, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::any_view<blocksci::Input, random_access_sized>> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::any_view<blocksci::Input>> && t);

    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::any_view<blocksci::Output>, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::any_view<blocksci::Output>> && t);

    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>, random_access_sized> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>> && t);

    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::any_view<blocksci::Block>, random_access_sized> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::any_view<blocksci::Block>> && t);

    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::any_view<blocksci::Address>, random_access_sized> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::any_view<blocksci::Address>> && t);

    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>> && t);

    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>, random_access_sized> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>, random_access_sized> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>> && t);
    
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<blocksci::OutputRange, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<blocksci::OutputRange> && t);
    
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<blocksci::InputRange, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<blocksci::InputRange> && t);

    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>, random_access_sized> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::any_view<int64_t>, random_access_sized> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::any_view<int64_t>> && t);


    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input>>, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input, random_access_sized>>> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input>>> && t);

    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output>>, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output, random_access_sized>>> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output>>> && t);

    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction>>, random_access_sized> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction, random_access_sized>>> && t);
    ranges::any_view<blocksci::Transaction> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction>>> && t);

    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block>>, random_access_sized> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block, random_access_sized>>> && t);
    ranges::any_view<blocksci::Block> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block>>> && t);

    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address>>, random_access_sized> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address, random_access_sized>>> && t);
    ranges::any_view<blocksci::Address> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address>>> && t);

    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript>>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript, random_access_sized>>> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript>>> && t);

    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress>>, random_access_sized> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress, random_access_sized>>> && t);
    ranges::any_view<blocksci::TaggedAddress> operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress>>> && t);

    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<blocksci::OutputRange>, random_access_sized> && t);
    ranges::any_view<blocksci::Output> operator()(ranges::any_view<ranges::optional<blocksci::OutputRange>> && t);

    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<blocksci::InputRange>, random_access_sized> && t);
    ranges::any_view<blocksci::Input> operator()(ranges::any_view<ranges::optional<blocksci::InputRange>> && t);

    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t>>, random_access_sized> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t, random_access_sized>>> && t);
    ranges::any_view<int64_t> operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t>>> && t);



    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>> && t);

    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>> && t);

    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>> && t);

    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>> && t);

    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>> && t);

    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>> && t);

    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>> && t);

    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>> && t);


    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::Input>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>>>> && t);

    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::Output>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>>>> && t);

    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::Transaction>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>>>> && t);

    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::Block>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>>>> && t);

    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::Address>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>>>> && t);

    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>>>> && t);

    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<blocksci::TaggedAddress>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>>> && t);

    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>, random_access_sized>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>>>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>, random_access_sized>>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>>>> && t);


    template <typename T, typename = std::enable_if_t<is_tagged<T>::value>>
    ranges::any_view<T, random_access_sized> operator()(ranges::any_view<T, random_access_sized> && t) {
        return t;
    }

    template <typename T, typename = std::enable_if_t<is_tagged<T>::value>>
    ranges::any_view<T> operator()(ranges::any_view<T> && t) {
        return t;
    }
};

constexpr ranges::category getBlockSciCategory(ranges::category cat) {
    if ((cat & random_access_sized) == random_access_sized) {
        return random_access_sized;
    } else {
        return ranges::category::input;
    }
}

template <typename T>
auto convertRangeToPython(T && t) {
    using V = ranges::range_value_type_t<T>;
    constexpr auto range_cat = getBlockSciCategory(ranges::get_categories<T>());
    auto converted = ConvertedRangeTypeTagConverter{}(ranges::any_view<V, range_cat>{t});
    return PythonConversionTypeConverter{}(std::move(converted));
}

#endif /* range_conversion_h */
