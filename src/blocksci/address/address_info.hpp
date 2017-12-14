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

#include <tuple>
#include <sstream>

namespace blocksci {
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEY> {
        static constexpr char name[] = "pubkey";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEYHASH> {
        static constexpr char name[] = "pubkeyhash";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_PUBKEYHASH> {
        static constexpr char name[] = "witness_pubkeyhash";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::SCRIPTHASH> {
        static constexpr char name[] = "scripthash";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_SCRIPTHASH> {
        static constexpr char name[] = "witness_scripthash";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::MULTISIG> {
        static constexpr char name[] = "multisig";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::MULTISIG;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NONSTANDARD> {
        static constexpr char name[] = "nonstandard";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::NONSTANDARD;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NULL_DATA> {
        static constexpr char name[] = "nulldata";
        static constexpr ScriptType::Enum scriptType = ScriptType::Enum::NULL_DATA;
    };
    
    using AddressInfoList = array_to_tuple_t<AddressType::Enum, AddressType::size, AddressType::all>;
    
    template <template<AddressType::Enum> class K>
    using to_address_tuple_t = apply_template_t<AddressType::Enum, K, AddressInfoList>;
    
    template<AddressType::Enum type>
    struct ScriptTypeFunctor {
        static constexpr ScriptType::Enum f() {
            return AddressInfo<type>::scriptType;
        }
    };
    
    static constexpr auto scriptTypeTable = blocksci::make_static_table<AddressType, ScriptTypeFunctor>();
    
    constexpr void addressTypeCheckThrow(size_t index) {  
        index >= AddressType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }

    constexpr ScriptType::Enum scriptType(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        addressTypeCheckThrow(index);
        return scriptTypeTable[index];
    }
    
    std::string addressName(AddressType::Enum type);
}

#endif /* address_info_hpp */
