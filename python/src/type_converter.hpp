//
//  type_converter.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef type_converter_h
#define type_converter_h

#include <blocksci/blocksci_fwd.hpp>

#include <range/v3/utility/optional.hpp>

#include <cstdint>


struct BasicTypeConverter {
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

    template <typename T>
    auto operator()(const ranges::optional<T> &val) -> ranges::optional<decltype(this->operator()(*val))> {
        if (val) {
            return this->operator()(*val);
        } else {
            return ranges::nullopt;
        }
    }

    template <typename A, typename B>
    auto operator()(const std::pair<A, B> &val) -> std::pair<decltype(this->operator()(val.first)), decltype(this->operator()(val.second))> {
        return {this->operator()(val.first), this->operator()(val.second)};
    }
};

#endif // type_converter_h