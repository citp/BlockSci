//
//  simplify_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "simplify_range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>


#include <range/v3/view/any_view.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

using namespace blocksci;

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

constexpr auto isOptional = [](const auto &optional) { return static_cast<bool>(optional); };
constexpr auto derefOptional = [](const auto &optional) { return *optional; };


template <typename T>
Iterator<typename ranges::range_value_t<T>::value_type> flattenOptional(T && t) {
    return {ranges::view::transform(ranges::view::filter(std::forward<T>(t), isOptional), derefOptional)};
}

template <typename T>
Iterator<typename ranges::range_value_t<ranges::range_value_t<T>>::value_type> flattenNestedOptional(T && t) {
	return flattenOptional(ranges::view::join(std::forward<T>(t)));
}


Range<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Input> && t) { return std::move(t); }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Input> && t) { return std::move(t); }

Range<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Output> && t) { return std::move(t); }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Output> && t) { return std::move(t); }

Range<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Transaction> && t) { return std::move(t); }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Transaction> && t) { return std::move(t); }

Range<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Block> && t) { return std::move(t); }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Block> && t) { return std::move(t); }

Range<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Address> && t) { return std::move(t); }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Address> && t) { return std::move(t); }

Range<blocksci::EquivAddress> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::EquivAddress> && t) { return std::move(t); }
Iterator<blocksci::EquivAddress> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::EquivAddress> && t) { return std::move(t); }

Range<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::AnyScript> && t) { return std::move(t); }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::AnyScript> && t) { return std::move(t); }

Range<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::TaggedAddress> && t) { return std::move(t); }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::TaggedAddress> && t) { return std::move(t); }

Range<blocksci::Cluster> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::Cluster> && t) { return std::move(t); }
Iterator<blocksci::Cluster> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::Cluster> && t) { return std::move(t); }

Range<blocksci::TaggedCluster> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::TaggedCluster> && t) { return std::move(t); }
Iterator<blocksci::TaggedCluster> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::TaggedCluster> && t) { return std::move(t); }

Range<int64_t> ConvertedRangeTypeTagConverter::operator()(Range<int64_t> && t) { return std::move(t); }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(Iterator<int64_t> && t) { return std::move(t); }

Range<bool> ConvertedRangeTypeTagConverter::operator()(Range<bool> && t) { return std::move(t); }
Iterator<bool> ConvertedRangeTypeTagConverter::operator()(Iterator<bool> && t) { return std::move(t); }

Range<blocksci::AddressType::Enum> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::AddressType::Enum> && t) { return std::move(t); }
Iterator<blocksci::AddressType::Enum> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::AddressType::Enum> && t) { return std::move(t); }

Range<blocksci::uint160> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::uint160> && t) { return std::move(t); }
Iterator<blocksci::uint160> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::uint160> && t) { return std::move(t); }

Range<blocksci::uint256> ConvertedRangeTypeTagConverter::operator()(Range<blocksci::uint256> && t) { return std::move(t); }
Iterator<blocksci::uint256> ConvertedRangeTypeTagConverter::operator()(Iterator<blocksci::uint256> && t) { return std::move(t); }

Range<pybind11::bytes> ConvertedRangeTypeTagConverter::operator()(Range<pybind11::bytes> && t) { return std::move(t); }
Iterator<pybind11::bytes> ConvertedRangeTypeTagConverter::operator()(Iterator<pybind11::bytes> && t) { return std::move(t); }

Range<pybind11::list> ConvertedRangeTypeTagConverter::operator()(Range<pybind11::list> && t) { return std::move(t); }
Iterator<pybind11::list> ConvertedRangeTypeTagConverter::operator()(Iterator<pybind11::list> && t) { return std::move(t); }

Range<std::chrono::system_clock::time_point> ConvertedRangeTypeTagConverter::operator()(Range<std::chrono::system_clock::time_point> && t) { return std::move(t); }
Iterator<std::chrono::system_clock::time_point> ConvertedRangeTypeTagConverter::operator()(Iterator<std::chrono::system_clock::time_point> && t) { return std::move(t); }

Range<std::string> ConvertedRangeTypeTagConverter::operator()(Range<std::string> && t) { return std::move(t); }
Iterator<std::string> ConvertedRangeTypeTagConverter::operator()(Iterator<std::string> && t) { return std::move(t); }




Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Input>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Input>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Output>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Output>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Transaction>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Transaction>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Block>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Block>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Address>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Address>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::EquivAddress> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::EquivAddress>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::EquivAddress> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::EquivAddress>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::AnyScript>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::AnyScript>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::TaggedAddress>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::TaggedAddress>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::Cluster> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::Cluster>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::Cluster> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::Cluster>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::TaggedCluster> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::TaggedCluster>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::TaggedCluster> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::TaggedCluster>> && t) { return flattenOptional(std::move(t)); }

Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<int64_t>> && t) { return flattenOptional(std::move(t)); }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<int64_t>> && t) { return flattenOptional(std::move(t)); }

Iterator<bool> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<bool>> && t) { return flattenOptional(std::move(t)); }
Iterator<bool> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<bool>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::AddressType::Enum> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::AddressType::Enum>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::AddressType::Enum> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::AddressType::Enum>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::uint160> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::uint160>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::uint160> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::uint160>> && t) { return flattenOptional(std::move(t)); }

Iterator<blocksci::uint256> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<blocksci::uint256>> && t) { return flattenOptional(std::move(t)); }
Iterator<blocksci::uint256> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<blocksci::uint256>> && t) { return flattenOptional(std::move(t)); }

Iterator<pybind11::bytes> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<pybind11::bytes>> && t) { return flattenOptional(std::move(t)); }
Iterator<pybind11::bytes> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<pybind11::bytes>> && t) { return flattenOptional(std::move(t)); }

Iterator<pybind11::list> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<pybind11::list>> && t) { return flattenOptional(std::move(t)); }
Iterator<pybind11::list> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<pybind11::list>> && t) { return flattenOptional(std::move(t)); }

Iterator<std::chrono::system_clock::time_point> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<std::chrono::system_clock::time_point>> && t) { return flattenOptional(std::move(t)); }
Iterator<std::chrono::system_clock::time_point> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<std::chrono::system_clock::time_point>> && t) { return flattenOptional(std::move(t)); }

Iterator<std::string> ConvertedRangeTypeTagConverter::operator()(Range<ranges::optional<std::string>> && t) { return flattenOptional(std::move(t)); }
Iterator<std::string> ConvertedRangeTypeTagConverter::operator()(Iterator<ranges::optional<std::string>> && t) { return flattenOptional(std::move(t)); }




Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Iterator<Iterator<blocksci::Input>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Iterator<Range<blocksci::Input>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Range<Range<blocksci::Input>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(Range<Iterator<blocksci::Input>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::OutputRange, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::OutputRange> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::InputRange, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::InputRange> && t) { return {ranges::view::join(std::move(t))}; }

Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t>> && t) { return {ranges::view::join(std::move(t))}; }



Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>> && t) { return flattenNestedOptional(std::move(t)); }

Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>> && t) { return flattenNestedOptional(std::move(t)); }
Iterator<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>> && t) { return flattenNestedOptional(std::move(t)); }
