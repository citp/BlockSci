//
//  script_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef script_info_hpp
#define script_info_hpp

#include "util.hpp"
#include "script_type.hpp"
#include <stdio.h>

namespace blocksci {
    struct PubkeyData;
    struct ScriptHashData;
    struct MultisigData;
    struct NonstandardScriptData;
    struct RawData;
    
    template <typename T>
    struct FixedSize;
    
    template <typename ...T>
    struct Indexed;
    
    template <ScriptType::Enum>
    struct ScriptInfo;
    
    template <>
    struct ScriptInfo<ScriptType::Enum::PUBKEY> {
        using storage = FixedSize<PubkeyData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::SCRIPTHASH> {
        using storage = FixedSize<ScriptHashData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::MULTISIG> {
        using storage = Indexed<MultisigData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::NONSTANDARD> {
        using storage = Indexed<NonstandardScriptData,NonstandardScriptData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::NULL_DATA> {
        using storage = Indexed<RawData>;
    };
    
    template<ScriptType::Enum ScriptType>
    struct ScriptTag {
        static constexpr ScriptType::Enum type = ScriptType;
    };
    
    namespace internal {
        template<template<ScriptType::Enum> class K, typename T>
        struct to_script_type;
        
        template<template<ScriptType::Enum> class K, ScriptType::Enum Type>
        struct to_script_type<K, ScriptTag<Type>> {
            using type = K<Type>;
        };
        
        template <template<ScriptType::Enum> class K, typename... Types>
        struct to_script_type<K, std::tuple<Types...>> {
            using type = std::tuple<typename to_script_type<K, Types>::type...>;
        };
    }
    
    inline auto getScriptTypes() {
        return internal::index_apply<ScriptType::all.size()>([](auto... Is) {
            return std::make_tuple(ScriptTag<std::get<Is>(ScriptType::all)>{}...);
        });
    }
    
    using ScriptInfoList = decltype(getScriptTypes());
}

#endif /* script_info_hpp */
