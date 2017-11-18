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

#include <blocksci/util/util.hpp>
#include <blocksci/scripts/script_type.hpp>

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
    
    using AddressInfoList = array_to_tuple_t<AddressType::Enum, AddressType::size, AddressType::all>;
    
    template <template<auto> class K>
    using to_address_tuple_t = apply_template_t<K, AddressInfoList>;
    
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
    
    constexpr std::string_view addressName(AddressType::Enum type) {
        auto index = static_cast<size_t>(type);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return addressNameTable[index];
    }
}

#endif /* address_info_hpp */
