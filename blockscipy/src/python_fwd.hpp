//
//  python_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/28/18.
//

#ifndef blocksci_python_fwd_h
#define blocksci_python_fwd_h

#include <cstdint>

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

struct BlocksciType;

template<typename T>
struct Sequence;

template <typename T>
struct Iterator;

template <typename T>
struct Range;

struct GenericIterator;
struct GenericRange;

struct TypenameLookup;


template <typename T, typename SimpleBase = SimpleProxy>
struct AllProxyClasses;

#endif /* blocksci_python_fwd_h */
