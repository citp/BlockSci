//
//  address_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef address_info_hpp
#define address_info_hpp

#include "dedup_address_type.hpp"

#include <blocksci/core/address_types.hpp>
#include <blocksci/core/meta.hpp>

#include <string>
#include <vector>

namespace blocksci {
    class uint160;
    class uint256;
    
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <>
    struct AddressInfo<AddressType::PUBKEY> {
        static constexpr char name[] = "pubkey";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::PUBKEYHASH> {
        static constexpr char name[] = "pubkeyhash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::MULTISIG_PUBKEY> {
        static constexpr char name[] = "multisig_pubkey";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::WITNESS_PUBKEYHASH> {
        static constexpr char name[] = "witness_pubkeyhash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::SCRIPTHASH> {
        static constexpr char name[] = "scripthash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
        static constexpr AddressType::Enum exampleType = AddressType::SCRIPTHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::WITNESS_SCRIPTHASH> {
        static constexpr char name[] = "witness_scripthash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
        static constexpr AddressType::Enum exampleType = AddressType::SCRIPTHASH;
        using IDType = uint256;
    };
    
    template <>
    struct AddressInfo<AddressType::MULTISIG> {
        static constexpr char name[] = "multisig";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::MULTISIG;
        static constexpr AddressType::Enum exampleType = AddressType::MULTISIG;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::NONSTANDARD> {
        static constexpr char name[] = "nonstandard";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::NONSTANDARD;
        static constexpr AddressType::Enum exampleType = AddressType::NONSTANDARD;
        using IDType = void;
    };
    
    template <>
    struct AddressInfo<AddressType::NULL_DATA> {
        static constexpr char name[] = "nulldata";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::NULL_DATA;
        static constexpr AddressType::Enum exampleType = AddressType::NULL_DATA;
        using IDType = void;
    };
    
    template <>
    struct AddressInfo<AddressType::WITNESS_UNKNOWN> {
        static constexpr char name[] = "witness_unknown";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::WITNESS_UNKNOWN;
        static constexpr AddressType::Enum exampleType = AddressType::WITNESS_UNKNOWN;
        using IDType = void;
    };
    
    template<AddressType::Enum type>
    struct DedupAddressTypeFunctor {
        static constexpr DedupAddressType::Enum f() {
            return AddressInfo<type>::dedupType;
        }
    };
    
    constexpr void addressTypeCheckThrow(size_t index) {  
        index >= AddressType::size ? throw std::invalid_argument("address type enum value is not valid") : 0;
    }

    constexpr DedupAddressType::Enum dedupType(AddressType::Enum t) {
        constexpr auto dedupTypeTable = blocksci::make_static_table<AddressType, DedupAddressTypeFunctor>();
        auto index = static_cast<size_t>(t);
        addressTypeCheckThrow(index);
        return dedupTypeTable[index];
    }
    
    std::string addressName(AddressType::Enum type);
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type);

#endif /* address_info_hpp */
