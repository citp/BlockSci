//
//  dynamic_table.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef dynamic_table_h
#define dynamic_table_h

#include <array>

namespace blocksci {
    namespace internal {
        template <typename EnumStruct, typename... Args>
        struct MakeDynamicTableImpl {
            template<template<typename EnumStruct::Enum, typename...> class Functor, std::size_t... Is>
            constexpr auto makeTable(std::index_sequence<Is...>) -> std::array<decltype(&Functor<EnumStruct::example, Args...>::f), sizeof...(Is)> {
                return {{Functor<std::tuple_element_t<Is, typename EnumStruct::all>::value, Args...>::f...}};
            }
        };
    } // namespace internal
    
    template <typename EnumStruct, template<typename EnumStruct::Enum, typename...> class Functor, typename... Args>
    constexpr auto make_dynamic_table() noexcept {
        internal::MakeDynamicTableImpl<EnumStruct, Args...> tableMaker{};
        return tableMaker.template makeTable<Functor>(std::make_index_sequence<EnumStruct::size>{});
    }
} // namespace blocksci

#endif /* dynamic_table_h */
