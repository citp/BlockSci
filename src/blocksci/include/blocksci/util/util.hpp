//
//  util.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef blocksci_util_util_h
#define blocksci_util_util_h

#include <tuple>
#include <utility>

namespace blocksci {
    namespace internal {
        template<typename A, template<typename...> class O>
        struct to_variadic;
        
        template<template<typename...> class V, template<typename...> class O, typename ...A>
        struct to_variadic<V<A...>, O> {
            using type = O<A...>;
        };
        
        template<template<class> class K, typename T>
        struct map_variadic;
        
        template <template<class> class K, typename... Types>
        struct map_variadic<K, std::tuple<Types...>> {
            using type = std::tuple<K<Types>...>;
        };
    }
    
    template<typename A, template<typename...> class O>
    using to_variadic_t = typename internal::to_variadic<A, O>::type;
}

#endif /* blocksci_util_util_h */
