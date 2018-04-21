//
//  static_table.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef static_table_h
#define static_table_h

#include <array>

namespace blocksci {
    namespace internal {
        template <typename EnumStruct, class ...Args>
        struct MakeStaticTableImpl {
            template<template<typename EnumStruct::Enum> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>, Args&&... args) -> std::array<decltype(Functor< EnumStruct::example>::f(std::forward<Args>(args)...)), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value>::f(std::forward<Args>(args)...)...}};
            }
        };
    } // namespace internal
    
    template <typename EnumStruct, template<typename EnumStruct::Enum> class Functor, class ...Args>
    constexpr auto make_static_table(Args&&... args) noexcept {
        internal::MakeStaticTableImpl<EnumStruct, Args...> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{}, std::forward<Args>(args)...);
    }
}

#endif /* static_table_h */
