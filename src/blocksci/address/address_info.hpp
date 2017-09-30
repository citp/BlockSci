//
//  address_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef address_info_hpp
#define address_info_hpp

#include "address_types.hpp"

#include <blocksci/util.hpp>
#include <blocksci/scripts/script_type.hpp>

#include <boost/variant/variant_fwd.hpp>

#include <string_view>
#include <tuple>
#include <sstream>

namespace blocksci {
    using namespace std::string_view_literals;
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEY> {
        static constexpr std::string_view name = "pubkey"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEYHASH> {
        static constexpr std::string_view name = "pubkeyhash"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_PUBKEYHASH> {
        static constexpr std::string_view name = "witness_pubkeyhash"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::SCRIPTHASH> {
        static constexpr std::string_view name = "scripthash"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_SCRIPTHASH> {
        static constexpr std::string_view name = "witness_scripthash"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::MULTISIG> {
        static constexpr std::string_view name = "multisig"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::MULTISIG;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NONSTANDARD> {
        static constexpr std::string_view name = "nonstandard"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::NONSTANDARD;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NULL_DATA> {
        static constexpr std::string_view name = "nulldata"sv;
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::NULL_DATA;
    };
    
    template<AddressType::Enum AddressType>
    struct AddressTag {
        static constexpr AddressType::Enum type = AddressType;
    };
    
    namespace internal {
        template<typename T>
        struct to_variant;
        
        template<typename... Ts>
        struct to_variant<std::tuple<Ts...>>
        {
            using type = boost::variant<Ts...>;
        };
        
        template<template<AddressType::Enum> class K, typename T>
        struct to_address_type;
        
        template<template<AddressType::Enum> class K, AddressType::Enum Type>
        struct to_address_type<K, AddressTag<Type>> {
            using type = K<Type>;
        };
        
        template <template<AddressType::Enum> class K, typename... Types>
        struct to_address_type<K, std::tuple<Types...>> {
            using type = std::tuple<typename to_address_type<K, Types>::type...>;
        };
    }
    
    template <template<AddressType::Enum> class K, typename... Types>
    using to_script_variant_t = typename internal::to_variant<typename internal::to_address_type<K, Types...>::type>::type;
    
    inline auto getAddressTypes() {
        return internal::index_apply<AddressType::all.size()>([](auto... Is) {
            return std::make_tuple(AddressTag<std::get<Is>(AddressType::all)>{}...);
        });
    }
    
    using AddressInfoList = decltype(getAddressTypes());
    
    template<AddressType::Enum type>
    struct ScriptTypeFunctor {
        static constexpr ScriptType::Enum f() {
            return AddressInfo<type>::scriptType;
        }
    };
    
    static constexpr auto scriptTypeTable = blocksci::make_static_table<AddressType, ScriptTypeFunctor>();
    
    constexpr ScriptType::Enum scriptType(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return scriptTypeTable[index];
    }
    
    template<AddressType::Enum type>
    struct TypeNameFunctor {
        static constexpr std::string_view f() {
            return AddressInfo<type>::name;
        }
    };
    
    static constexpr auto addressNameTable = make_static_table<AddressType, TypeNameFunctor>();
    
    constexpr std::string_view GetTxnOutputType(AddressType::Enum type) {
        auto index = static_cast<size_t>(type);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return addressNameTable[index];
    }
}

#endif /* address_info_hpp */
