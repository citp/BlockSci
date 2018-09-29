//
//  simplify_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef simplify_range_h
#define simplify_range_h

#include "range_utils.hpp"

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/chain/input.hpp>

#include <pybind11/numpy.h>

#include <range/v3/view/any_view.hpp>

#include <chrono>

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

template <typename T>
constexpr bool isNonTaggedRange() {
    return !is_tagged<T>::value && ranges::Range<T>();
}

struct ConvertedRangeTypeTagConverter {
    Range<blocksci::Input> operator()(Range<blocksci::Input> && t);
    Iterator<blocksci::Input> operator()(Iterator<blocksci::Input> && t);

    Range<blocksci::Output> operator()(Range<blocksci::Output> && t);
    Iterator<blocksci::Output> operator()(Iterator<blocksci::Output> && t);

    Range<blocksci::Transaction> operator()(Range<blocksci::Transaction> && t);
    Iterator<blocksci::Transaction> operator()(Iterator<blocksci::Transaction> && t);

    Range<blocksci::Block> operator()(Range<blocksci::Block> && t);
    Iterator<blocksci::Block> operator()(Iterator<blocksci::Block> && t);

    Range<blocksci::Address> operator()(Range<blocksci::Address> && t);
    Iterator<blocksci::Address> operator()(Iterator<blocksci::Address> && t);

    Range<blocksci::EquivAddress> operator()(Range<blocksci::EquivAddress> && t);
    Iterator<blocksci::EquivAddress> operator()(Iterator<blocksci::EquivAddress> && t);

    Range<blocksci::AnyScript> operator()(Range<blocksci::AnyScript> && t);
    Iterator<blocksci::AnyScript> operator()(Iterator<blocksci::AnyScript> && t);

    Range<blocksci::TaggedAddress> operator()(Range<blocksci::TaggedAddress> && t);
    Iterator<blocksci::TaggedAddress> operator()(Iterator<blocksci::TaggedAddress> && t);

    Range<blocksci::Cluster> operator()(Range<blocksci::Cluster> && t);
    Iterator<blocksci::Cluster> operator()(Iterator<blocksci::Cluster> && t);

    Range<blocksci::TaggedCluster> operator()(Range<blocksci::TaggedCluster> && t);
    Iterator<blocksci::TaggedCluster> operator()(Iterator<blocksci::TaggedCluster> && t);

    Range<int64_t> operator()(Range<int64_t> && t);
    Iterator<int64_t> operator()(Iterator<int64_t> && t);

    Range<bool> operator()(Range<bool> && t);
    Iterator<bool> operator()(Iterator<bool> && t);

    Range<blocksci::AddressType::Enum> operator()(Range<blocksci::AddressType::Enum> && t);
    Iterator<blocksci::AddressType::Enum> operator()(Iterator<blocksci::AddressType::Enum> && t);

    Range<blocksci::uint160> operator()(Range<blocksci::uint160> && t);
    Iterator<blocksci::uint160> operator()(Iterator<blocksci::uint160> && t);

    Range<blocksci::uint256> operator()(Range<blocksci::uint256> && t);
    Iterator<blocksci::uint256> operator()(Iterator<blocksci::uint256> && t);

    Range<pybind11::bytes> operator()(Range<pybind11::bytes> && t);
    Iterator<pybind11::bytes> operator()(Iterator<pybind11::bytes> && t);

    Range<pybind11::list> operator()(Range<pybind11::list> && t);
    Iterator<pybind11::list> operator()(Iterator<pybind11::list> && t);

    Range<std::chrono::system_clock::time_point> operator()(Range<std::chrono::system_clock::time_point> && t);
    Iterator<std::chrono::system_clock::time_point> operator()(Iterator<std::chrono::system_clock::time_point> && t);

    Range<std::string> operator()(Range<std::string> && t);
    Iterator<std::string> operator()(Iterator<std::string> && t);




    Iterator<blocksci::Input> operator()(Range<ranges::optional<blocksci::Input>> && t);
    Iterator<blocksci::Input> operator()(Iterator<ranges::optional<blocksci::Input>> && t);

    Iterator<blocksci::Output> operator()(Range<ranges::optional<blocksci::Output>> && t);
    Iterator<blocksci::Output> operator()(Iterator<ranges::optional<blocksci::Output>> && t);

    Iterator<blocksci::Transaction> operator()(Range<ranges::optional<blocksci::Transaction>> && t);
    Iterator<blocksci::Transaction> operator()(Iterator<ranges::optional<blocksci::Transaction>> && t);

    Iterator<blocksci::Block> operator()(Range<ranges::optional<blocksci::Block>> && t);
    Iterator<blocksci::Block> operator()(Iterator<ranges::optional<blocksci::Block>> && t);

    Iterator<blocksci::Address> operator()(Range<ranges::optional<blocksci::Address>> && t);
    Iterator<blocksci::Address> operator()(Iterator<ranges::optional<blocksci::Address>> && t);

    Iterator<blocksci::EquivAddress> operator()(Range<ranges::optional<blocksci::EquivAddress>> && t);
    Iterator<blocksci::EquivAddress> operator()(Iterator<ranges::optional<blocksci::EquivAddress>> && t);

    Iterator<blocksci::AnyScript> operator()(Range<ranges::optional<blocksci::AnyScript>> && t);
    Iterator<blocksci::AnyScript> operator()(Iterator<ranges::optional<blocksci::AnyScript>> && t);

    Iterator<blocksci::TaggedAddress> operator()(Range<ranges::optional<blocksci::TaggedAddress>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Iterator<ranges::optional<blocksci::TaggedAddress>> && t);

    Iterator<blocksci::Cluster> operator()(Range<ranges::optional<blocksci::Cluster>> && t);
    Iterator<blocksci::Cluster> operator()(Iterator<ranges::optional<blocksci::Cluster>> && t);

    Iterator<blocksci::TaggedCluster> operator()(Range<ranges::optional<blocksci::TaggedCluster>> && t);
    Iterator<blocksci::TaggedCluster> operator()(Iterator<ranges::optional<blocksci::TaggedCluster>> && t);

    Iterator<int64_t> operator()(Range<ranges::optional<int64_t>> && t);
    Iterator<int64_t> operator()(Iterator<ranges::optional<int64_t>> && t);

    Iterator<bool> operator()(Range<ranges::optional<bool>> && t);
    Iterator<bool> operator()(Iterator<ranges::optional<bool>> && t);

    Iterator<blocksci::AddressType::Enum> operator()(Range<ranges::optional<blocksci::AddressType::Enum>> && t);
    Iterator<blocksci::AddressType::Enum> operator()(Iterator<ranges::optional<blocksci::AddressType::Enum>> && t);

    Iterator<blocksci::uint160> operator()(Range<ranges::optional<blocksci::uint160>> && t);
    Iterator<blocksci::uint160> operator()(Iterator<ranges::optional<blocksci::uint160>> && t);

    Iterator<blocksci::uint256> operator()(Range<ranges::optional<blocksci::uint256>> && t);
    Iterator<blocksci::uint256> operator()(Iterator<ranges::optional<blocksci::uint256>> && t);

    Iterator<pybind11::bytes> operator()(Range<ranges::optional<pybind11::bytes>> && t);
    Iterator<pybind11::bytes> operator()(Iterator<ranges::optional<pybind11::bytes>> && t);

    Iterator<pybind11::list> operator()(Range<ranges::optional<pybind11::list>> && t);
    Iterator<pybind11::list> operator()(Iterator<ranges::optional<pybind11::list>> && t);

    Iterator<std::chrono::system_clock::time_point> operator()(Range<ranges::optional<std::chrono::system_clock::time_point>> && t);
    Iterator<std::chrono::system_clock::time_point> operator()(Iterator<ranges::optional<std::chrono::system_clock::time_point>> && t);

    Iterator<std::string> operator()(Range<ranges::optional<std::string>> && t);
    Iterator<std::string> operator()(Iterator<ranges::optional<std::string>> && t);




    Iterator<blocksci::Input> operator()(Iterator<Iterator<blocksci::Input>> && t);
    Iterator<blocksci::Input> operator()(Iterator<Range<blocksci::Input>> && t);
    Iterator<blocksci::Input> operator()(Range<Range<blocksci::Input>> && t);
    Iterator<blocksci::Input> operator()(Range<Iterator<blocksci::Input>> && t);

    Iterator<blocksci::Output> operator()(Iterator<Iterator<blocksci::Output>> && t);
    Iterator<blocksci::Output> operator()(Iterator<Range<blocksci::Output>> && t);
    Iterator<blocksci::Output> operator()(Range<Range<blocksci::Output>> && t);
    Iterator<blocksci::Output> operator()(Range<Iterator<blocksci::Output>> && t);

    Iterator<blocksci::Transaction> operator()(Iterator<Iterator<blocksci::Transaction>> && t);
    Iterator<blocksci::Transaction> operator()(Iterator<Range<blocksci::Transaction>> && t);
    Iterator<blocksci::Transaction> operator()(Range<Range<blocksci::Transaction>> && t);
    Iterator<blocksci::Transaction> operator()(Range<Iterator<blocksci::Transaction>> && t);

    Iterator<blocksci::Block> operator()(Iterator<Iterator<blocksci::Block>> && t);
    Iterator<blocksci::Block> operator()(Iterator<Range<blocksci::Block>> && t);
    Iterator<blocksci::Block> operator()(Range<Range<blocksci::Block>> && t);
    Iterator<blocksci::Block> operator()(Range<Iterator<blocksci::Block>> && t);

    Iterator<blocksci::Address> operator()(Iterator<Iterator<blocksci::Address>> && t);
    Iterator<blocksci::Address> operator()(Iterator<Range<blocksci::Address>> && t);
    Iterator<blocksci::Address> operator()(Range<Range<blocksci::Address>> && t);
    Iterator<blocksci::Address> operator()(Range<Iterator<blocksci::Address>> && t);

    Iterator<blocksci::AnyScript> operator()(Iterator<Iterator<blocksci::AnyScript>> && t);
    Iterator<blocksci::AnyScript> operator()(Iterator<Range<blocksci::AnyScript>> && t);
    Iterator<blocksci::AnyScript> operator()(Range<Range<blocksci::AnyScript>> && t);
    Iterator<blocksci::AnyScript> operator()(Range<Iterator<blocksci::AnyScript>> && t);

    Iterator<blocksci::TaggedAddress> operator()(Iterator<Iterator<blocksci::TaggedAddress>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Iterator<Range<blocksci::TaggedAddress>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Range<Range<blocksci::TaggedAddress>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Range<Iterator<blocksci::TaggedAddress>> && t);

    Iterator<blocksci::Output> operator()(Iterator<blocksci::OutputRange> && t);
    Iterator<blocksci::Output> operator()(Range<blocksci::OutputRange> && t);
    
    Iterator<blocksci::Input> operator()(Iterator<blocksci::InputRange> && t);
    Iterator<blocksci::Input> operator()(Range<blocksci::InputRange> && t);

    Iterator<int64_t> operator()(Iterator<Iterator<int64_t>> && t);
    Iterator<int64_t> operator()(Iterator<Range<int64_t>> && t);
    Iterator<int64_t> operator()(Range<Range<int64_t>> && t);
    Iterator<int64_t> operator()(Range<Iterator<int64_t>> && t);


    Iterator<blocksci::Input> operator()(Iterator<Iterator<ranges::optional<blocksci::Input>>> && t);
    Iterator<blocksci::Input> operator()(Iterator<Range<ranges::optional<blocksci::Input>>> && t);
    Iterator<blocksci::Input> operator()(Range<Range<ranges::optional<blocksci::Input>>> && t);
    Iterator<blocksci::Input> operator()(Range<Iterator<ranges::optional<blocksci::Input>>> && t);

    Iterator<blocksci::Output> operator()(Iterator<Iterator<ranges::optional<blocksci::Output>>> && t);
    Iterator<blocksci::Output> operator()(Iterator<Range<ranges::optional<blocksci::Output>>> && t);
    Iterator<blocksci::Output> operator()(Range<Range<ranges::optional<blocksci::Output>>> && t);
    Iterator<blocksci::Output> operator()(Range<Iterator<ranges::optional<blocksci::Output>>> && t);

    Iterator<blocksci::Transaction> operator()(Iterator<Iterator<ranges::optional<blocksci::Transaction>>> && t);
    Iterator<blocksci::Transaction> operator()(Iterator<Range<ranges::optional<blocksci::Transaction>>> && t);
    Iterator<blocksci::Transaction> operator()(Range<Range<ranges::optional<blocksci::Transaction>>> && t);
    Iterator<blocksci::Transaction> operator()(Range<Iterator<ranges::optional<blocksci::Transaction>>> && t);

    Iterator<blocksci::Block> operator()(Iterator<Iterator<ranges::optional<blocksci::Block>>> && t);
    Iterator<blocksci::Block> operator()(Iterator<Range<ranges::optional<blocksci::Block>>> && t);
    Iterator<blocksci::Block> operator()(Range<Range<ranges::optional<blocksci::Block>>> && t);
    Iterator<blocksci::Block> operator()(Range<Iterator<ranges::optional<blocksci::Block>>> && t);

    Iterator<blocksci::Address> operator()(Iterator<Iterator<ranges::optional<blocksci::Address>>> && t);
    Iterator<blocksci::Address> operator()(Iterator<Range<ranges::optional<blocksci::Address>>> && t);
    Iterator<blocksci::Address> operator()(Range<Range<ranges::optional<blocksci::Address>>> && t);
    Iterator<blocksci::Address> operator()(Range<Iterator<ranges::optional<blocksci::Address>>> && t);

    Iterator<blocksci::AnyScript> operator()(Iterator<Iterator<ranges::optional<blocksci::AnyScript>>> && t);
    Iterator<blocksci::AnyScript> operator()(Iterator<Range<ranges::optional<blocksci::AnyScript>>> && t);
    Iterator<blocksci::AnyScript> operator()(Range<Range<ranges::optional<blocksci::AnyScript>>> && t);
    Iterator<blocksci::AnyScript> operator()(Range<Iterator<ranges::optional<blocksci::AnyScript>>> && t);

    Iterator<blocksci::TaggedAddress> operator()(Iterator<Iterator<ranges::optional<blocksci::TaggedAddress>>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Iterator<Range<ranges::optional<blocksci::TaggedAddress>>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Range<Range<ranges::optional<blocksci::TaggedAddress>>> && t);
    Iterator<blocksci::TaggedAddress> operator()(Range<Iterator<ranges::optional<blocksci::TaggedAddress>>> && t);

    Iterator<int64_t> operator()(Iterator<Iterator<ranges::optional<int64_t>>> && t);
    Iterator<int64_t> operator()(Iterator<Range<ranges::optional<int64_t>>> && t);
    Iterator<int64_t> operator()(Range<Range<ranges::optional<int64_t>>> && t);
    Iterator<int64_t> operator()(Range<Iterator<ranges::optional<int64_t>>> && t);
};

constexpr ranges::category getBlockSciCategory(ranges::category cat) {
    if ((cat & random_access_sized) == random_access_sized) {
        return random_access_sized;
    } else {
        return ranges::category::input;
    }
}

template <typename T>
auto simplifyRange(T && t) {
    using V = ranges::range_value_type_t<T>;
    constexpr auto range_cat = getBlockSciCategory(ranges::get_categories<T>());
    return ConvertedRangeTypeTagConverter{}(ranges::any_view<V, range_cat>{t});
}

#endif /* simplify_range_h */
