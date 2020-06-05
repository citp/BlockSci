//
//  python_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/28/18.
//

#ifndef blocksci_python_fwd_h
#define blocksci_python_fwd_h

#include <range/v3/view/any_view.hpp>
#include <cstdint>

enum class ProxyType {
	Simple, Optional, Iterator, Range
};

struct ProxyTypeInfo;
struct BlocksciType;
struct BlocksciIteratorType;
struct BlocksciRangeType;

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

template <typename T>
using RawIterator = ranges::any_view<T>;

template <typename T>
using RawRange = ranges::any_view<T, random_access_sized>;

template <typename T>
struct RangeClasses;

template<typename T>
struct Proxy;

struct NumpyBool {
    bool val;
};

struct NumpyDatetime {
    int64_t time;
};

struct GenericProxy;
struct SimpleProxy;
struct IteratorProxy;
struct RangeProxy;
struct OptionalProxy;
struct ProxyAddress;

template <typename T>
struct SequenceProxy;

template<typename T>
struct Sequence;

template <typename T>
struct Iterator;

template <typename T>
struct Range;

struct GenericIterator;
struct GenericRange;

struct GenericAddressIterator;
struct GenericAddressRange;

struct TypenameLookup;


template <typename T, typename SimpleBase = SimpleProxy>
struct AllProxyClasses;

#endif /* blocksci_python_fwd_h */
