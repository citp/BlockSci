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
        
        template<int I, class Tuple, typename F> struct for_each_impl {
            static constexpr void for_each(Tuple&& t, F f) {
                for_each_impl<I - 1, Tuple, F>::for_each(std::forward<Tuple>(t), f);
                f(std::get<I>(t));
            }
        };
        template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
            static constexpr void for_each(Tuple&& t, F f) {
                f(std::get<0>(std::forward<Tuple>(t)));
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
            constexpr auto makeTable(std::index_sequence<Is...>) -> std::array<decltype(&Functor<EnumStruct::example>::f), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value>::f...}};
            }
        };
        
        
        template <typename EnumStruct, class ...Args>
        struct MakeStaticTableImpl {
            template<template<typename EnumStruct::Enum> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>, Args&&... args) -> std::array<decltype(Functor< EnumStruct::example>::f(std::forward<Args>(args)...)), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value>::f(std::forward<Args>(args)...)...}};
            }
        };
        
        template<typename A, template<typename...> class O>
        struct to_variadic;
        
        template<template<typename...> class V, template<typename...> class O, typename ...A>
        struct to_variadic<V<A...>, O> {
            using type = O<A...>;
        };
        
        
        template<typename T, size_t i, const std::array<T, i> &A>
        struct TupGen {
            static auto f() {
                return internal::index_apply<A.size()>([](auto... Is) {
                    return std::make_tuple(std::integral_constant<T, std::get<Is>(A)>{}...);
                });
            }
        };
        
        template<typename Z, template<Z> class K, typename T>
        struct apply_template;
        
        template <typename Z, template<Z> class K, typename... Types>
        struct apply_template<Z, K, std::tuple<Types...>> {
            using type = std::tuple<K<Types::value>...>;
        };
        
        template<template<class> class K, typename T>
        struct map_variadic;
        
        template <template<class> class K, typename... Types>
        struct map_variadic<K, std::tuple<Types...>> {
            using type = std::tuple<K<Types>...>;
        };
    }
    
    template<typename T, size_t i, const std::array<T, i> &A>
    using array_to_tuple_t = decltype(internal::TupGen<T, i, A>::f());
    
    template <typename T, template<T> class K, typename... Types>
    using apply_template_t = typename internal::apply_template<T, K, Types...>::type;
    
    template<typename A, template<typename...> class O>
    using to_variadic_t = typename internal::to_variadic<A, O>::type;
    
    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor, class ...Args>
    constexpr auto make_static_table(Args&&... args) noexcept {
        internal::MakeStaticTableImpl<EnumStruct, Args...> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{}, std::forward<Args>(args)...);
    }
    
    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor>
    constexpr auto make_dynamic_table() noexcept {
        internal::MakeDynamicTableImpl<EnumStruct> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{});
    }
    
    template<class Tuple, typename F>
    constexpr void for_each(Tuple&& t, F f) {
        internal::for_each_impl<std::tuple_size<typename std::remove_reference<Tuple>::type>::value - 1, Tuple, F>::for_each(std::forward<Tuple>(t), f);
    }
    
    template <class Tuple, class F>
    constexpr auto apply(Tuple t, F f) {
        return internal::index_apply<std::tuple_size<Tuple>{}>([&](auto... Is) {
            return std::make_tuple(f(std::get<Is>(t))...);
        });
    }
    
    template <typename T, typename Index = uint32_t>
    class InPlaceArray {
        Index dataSize;
        
    public:
        explicit InPlaceArray(Index size) : dataSize(size) {}
        
        Index size() const {
            return dataSize;
        }
        
        T &operator[](Index index) {
            return *(reinterpret_cast<T *>(reinterpret_cast<char *>(this) + sizeof(InPlaceArray)) + index);
        }
        
        const T &operator[](Index index) const {
            return *(reinterpret_cast<const T *>(reinterpret_cast<const char *>(this) + sizeof(InPlaceArray)) + index);
        }
        
        const T *begin() const {
            return &operator[](Index{0});
        }
        
        const T *end() const {
            return &operator[](size());
        }
        
        const T *begin() {
            return &operator[](Index{0});
        }
        
        const T *end() {
            return &operator[](size());
        }
        
        size_t extraSize() const {
            return sizeof(T) * size();
        }
        
    };
}

#endif /* blocksci_util_util_h */
