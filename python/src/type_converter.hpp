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

template <typename T>
struct BasicTypeConverter {
    using type = T;
    type operator()(const T &val) {
        return val;
    }
};

template <typename T>
struct BasicTypeConverter<ranges::optional<T>> {
    using type = ranges::optional<typename BasicTypeConverter<T>::type>;
    type operator()(const ranges::optional<T> &val) {
        if (val) {
            return BasicTypeConverter<T>{}(*val);
        } else {
            return ranges::nullopt;
        }
    }
};

template <typename A, typename B>
struct BasicTypeConverter<std::pair<A, B>> {
    using typeA = typename BasicTypeConverter<A>::type;
    using typeB = typename BasicTypeConverter<B>::type;
    using type = std::pair<typeA, typeB>;
    type operator()(const std::pair<A, B> &val) {
        return {BasicTypeConverter<A>{}(val.first), BasicTypeConverter<B>{}(val.second)};
    }
};

template<>
struct BasicTypeConverter<blocksci::Address> {
    using type = blocksci::AnyScript;
    type operator()(const blocksci::Address &address);
};

template<>
struct BasicTypeConverter<uint16_t> {
    using type = int64_t;
    type operator()(uint16_t val) {
        return static_cast<type>(val);
    }
};

template<>
struct BasicTypeConverter<int16_t> {
    using type = int64_t;
    type operator()(int16_t val) {
        return static_cast<type>(val);
    }
};

template<>
struct BasicTypeConverter<int> {
    using type = int64_t;
    type operator()(int val) {
        return static_cast<type>(val);
    }
};

template<>
struct BasicTypeConverter<unsigned int> {
    using type = int64_t;
    type operator()(unsigned int val) {
        return static_cast<type>(val);
    }
};

template<>
struct BasicTypeConverter<long> {
    using type = int64_t;
    type operator()(long val) {
        return static_cast<type>(val);
    }
};

template<>
struct BasicTypeConverter<int64_t> {
    using type = int64_t;
    type operator()(int64_t val) {
        return static_cast<type>(val);
    }
};

// Potential overflow
template<>
struct BasicTypeConverter<uint64_t> {
    using type = int64_t;
    type operator()(uint64_t val) {
        return static_cast<type>(val);
    }
};

#endif // type_converter_h