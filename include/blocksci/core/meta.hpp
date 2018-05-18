//
//  meta.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/22/18.
//

#ifndef blocksci_meta_h
#define blocksci_meta_h

#include <array>
#include <tuple>

namespace blocksci {
    namespace internal {
        template <class F, size_t... Is>
        constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
            return f(std::integral_constant<size_t, Is> {}...);
        }
        
        template <size_t N, class F>
        constexpr auto index_apply(F f) {
            return index_apply_impl(f, std::make_index_sequence<N>{});
        }

        template<typename A, template<typename...> class O>
        struct to_variadic;
        
        template<template<typename...> class V, template<typename...> class O, typename ...A>
        struct to_variadic<V<A...>, O> {
            using type = O<A...>;
        };

        template <typename EnumStruct, typename... Args>
        struct MakeDynamicTableImpl {
            template<template<typename EnumStruct::Enum, typename...> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>) -> std::array<decltype(&Functor<EnumStruct::example, Args...>::f), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value, Args...>::f...}};
            }
        };

        template <typename EnumStruct, class ...Args>
        struct MakeStaticTableImpl {
            template<template<typename EnumStruct::Enum> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>, Args&&... args) -> std::array<decltype(Functor< EnumStruct::example>::f(std::forward<Args>(args)...)), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value>::f(std::forward<Args>(args)...)...}};
            }
        };

        template<typename Z, template<Z> class K, typename T>
        struct apply_template;
        
        template <typename Z, template<Z> class K, typename... Types>
        struct apply_template<Z, K, std::tuple<Types...>> {
            using type = std::tuple<K<Types::value>...>;
        };
    } // namespace internal
    
    template <class Tuple, class F>
    constexpr auto apply(Tuple && t, F f) {
        return internal::index_apply<std::tuple_size<std::decay_t<Tuple>>{}>([&](auto... Is) {
            return std::make_tuple(f(std::get<Is>(t))...);
        });
    }
    
    template<class Tuple, typename F>
    constexpr void for_each(Tuple&& t, F f) {
        blocksci::apply(std::forward<Tuple>(t), [&f](auto && item) {
            f(item);
            return 1;
        });
    }

    template<typename A, template<typename...> class O>
    using to_variadic_t = typename internal::to_variadic<A, O>::type;

    template <typename EnumStruct, template<typename EnumStruct::Enum, typename...> class Functor, typename... Args>
    constexpr auto make_dynamic_table() noexcept {
        internal::MakeDynamicTableImpl<EnumStruct, Args...> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{});
    }

    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor, class ...Args>
    constexpr auto make_static_table(Args&&... args) noexcept {
        internal::MakeStaticTableImpl<EnumStruct, Args...> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{}, std::forward<Args>(args)...);
    }

    template <typename T, template<T> class K, typename... Types>
    using apply_template_t = typename internal::apply_template<T, K, Types...>::type;
} // namespace blocksci

#endif /* blocksci_meta_h */
