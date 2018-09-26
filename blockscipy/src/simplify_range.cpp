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
ranges::any_view<ranges::range_value_t<typename ranges::range_value_t<T>::value_type>> flattenNestedOptional(T && t) {
    return {ranges::view::join(ranges::view::transform(ranges::view::filter(std::forward<T>(t), isOptional), derefOptional))};
}

ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Input, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Input>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Input, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Input>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Output>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Transaction>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Block>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::Address>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::AnyScript>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<blocksci::TaggedAddress>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::OutputRange, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::OutputRange> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::InputRange, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<blocksci::InputRange> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<int64_t>> && t) { return {ranges::view::join(std::move(t))}; }


ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Input>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Output>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Transaction> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Transaction>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Block> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Block>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Address> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::Address>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::AnyScript> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::AnyScript>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::TaggedAddress> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<blocksci::TaggedAddress>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<blocksci::OutputRange>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Output> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<blocksci::OutputRange>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<blocksci::InputRange>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<blocksci::Input> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<blocksci::InputRange>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<int64_t> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<int64_t>>> && t) { return flattenNestedOptional(std::move(t)); }


ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Input>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Output>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Transaction>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Block>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::Address>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::AnyScript>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>> && t) { return {ranges::view::join(std::move(t))}; }

ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>, random_access_sized> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>, random_access_sized>> && t) { return {ranges::view::join(std::move(t))}; }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::any_view<ranges::optional<int64_t>>> && t) { return {ranges::view::join(std::move(t))}; }


ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Input>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Input>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Output>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Output>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Transaction>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Transaction>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Block>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Block>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::Address>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::Address>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::AnyScript>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::AnyScript>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<blocksci::TaggedAddress>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<blocksci::TaggedAddress>>>> && t) { return flattenNestedOptional(std::move(t)); }

ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>, random_access_sized>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>>>, random_access_sized> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>, random_access_sized>>> && t) { return flattenNestedOptional(std::move(t)); }
ranges::any_view<ranges::optional<int64_t>> ConvertedRangeTypeTagConverter::operator()(ranges::any_view<ranges::optional<ranges::any_view<ranges::optional<int64_t>>>> && t) { return flattenNestedOptional(std::move(t)); }
