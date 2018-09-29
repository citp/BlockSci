//
//  blocksci_type_converter.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_type_converter_h
#define blocksci_type_converter_h

#include "range_utils.hpp"
#include <blocksci/chain/block_range.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/transform.hpp>

struct BlockSciTypeConverter {

    template <typename T>
    T operator()(const T &t) {
        return t;
    }

    blocksci::AnyScript operator()(const blocksci::Address &address) {
        return address.getScript();
    }

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

    Range<blocksci::Input> operator()(const blocksci::InputRange &val) {
        return val;
    }

    Range<blocksci::Output> operator()(const blocksci::OutputRange &val) {
        return val;
    }

    Range<blocksci::Block> operator()(const blocksci::BlockRange &val) {
        return val;
    }

    template <typename T>
    auto operator()(const ranges::optional<T> &val) -> ranges::optional<decltype(this->operator()(*val))> {
        if (val) {
            return this->operator()(*val);
        } else {
            return ranges::nullopt;
        }
    }

    template <typename T>
    auto operator()(Iterator<T> &val) -> Iterator<decltype(this->operator()(*val))> {
        return val | ranges::view::transform(this);
    }

    template <typename T>
    auto operator()(Range<T> &val) -> Range<decltype(this->operator()(*val))> {
        return val | ranges::view::transform(this);
    }
};

#endif /* blocksci_type_converter_h */