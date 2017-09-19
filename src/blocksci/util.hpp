//
//  util.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef util_h
#define util_h

#include <tuple>
#include <utility>

namespace blocksci {
    namespace internal {
        
        template<int I, class Tuple, typename F> struct for_each_impl {
            static void for_each(Tuple& t, F f) {
                for_each_impl<I - 1, Tuple, F>::for_each(t, f);
                f(std::get<I>(t));
            }
        };
        template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
            static void for_each(Tuple& t, F f) {
                f(std::get<0>(t));
            }
        };
        
        template <class F, size_t... Is>
        constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
            return f(std::integral_constant<size_t, Is> {}...);
        }
        
        template <size_t N, class F>
        constexpr auto index_apply(F f) {
            return index_apply_impl(f, std::make_index_sequence<N>{});
        }
        
        template <typename EnumStruct>
        struct MakeDynamicTableImpl {
            template<template<typename EnumStruct::Enum> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>) -> std::array<decltype(&Functor<EnumStruct::all[0]>::f), sizeof...(Is)> {
                return {{Functor<EnumStruct::all[Is]>::f...}};
            }
        };
        
        
        template <typename EnumStruct, class ...Args>
        struct MakeStaticTableImpl {
            template<template<typename EnumStruct::Enum> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>, Args... args) -> std::array<decltype(Functor<EnumStruct::all[0]>::f(args...)), sizeof...(Is)> {
                return {{Functor<EnumStruct::all[Is]>::f(args...)...}};
            }
        };
    }
    
    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor, class ...Args>
    constexpr auto make_static_table(Args... args) {
        internal::MakeStaticTableImpl<EnumStruct, Args...> tableMaker;
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::all.size()>{}, args...);
    }
    
    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor>
    constexpr auto make_dynamic_table() {
        internal::MakeDynamicTableImpl<EnumStruct> tableMaker;
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::all.size()>{});
    }
    
    template<class Tuple, typename F>
    void for_each(Tuple& t, F f) {
        internal::for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F>::for_each(t, f);
    }
    
    template <class Tuple, class F>
    constexpr auto apply(Tuple t, F f) {
        return internal::index_apply<std::tuple_size<Tuple>{}>([&](auto... Is) {
            return std::make_tuple(f(std::get<Is>(t))...);
        });
    }
}

#endif /* util_h */
