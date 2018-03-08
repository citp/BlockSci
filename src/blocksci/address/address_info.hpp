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
#include "dedup_address_info.hpp"

#include <blocksci/util/util.hpp>

#include <tuple>
#include <sstream>

namespace blocksci {
    class uint160;
    class uint256;
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEY> {
        static constexpr char name[] = "pubkey";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEYHASH> {
        static constexpr char name[] = "pubkeyhash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_PUBKEYHASH> {
        static constexpr char name[] = "witness_pubkeyhash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
        static constexpr AddressType::Enum exampleType = AddressType::PUBKEYHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::SCRIPTHASH> {
        static constexpr char name[] = "scripthash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
        static constexpr AddressType::Enum exampleType = AddressType::SCRIPTHASH;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::WITNESS_SCRIPTHASH> {
        static constexpr char name[] = "witness_scripthash";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
        static constexpr AddressType::Enum exampleType = AddressType::SCRIPTHASH;
        using IDType = uint256;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::MULTISIG> {
        static constexpr char name[] = "multisig";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::MULTISIG;
        static constexpr AddressType::Enum exampleType = AddressType::MULTISIG;
        using IDType = uint160;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NONSTANDARD> {
        static constexpr char name[] = "nonstandard";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::NONSTANDARD;
        static constexpr AddressType::Enum exampleType = AddressType::NONSTANDARD;
        using IDType = void;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NULL_DATA> {
        static constexpr char name[] = "nulldata";
        static constexpr DedupAddressType::Enum dedupType = DedupAddressType::NULL_DATA;
        static constexpr AddressType::Enum exampleType = AddressType::NULL_DATA;
        using IDType = void;
    };
    
    using AddressInfoList = array_to_tuple_t<AddressType::Enum, AddressType::size, AddressType::all>;
    
    template <template<AddressType::Enum> class K>
    using to_address_tuple_t = apply_template_t<AddressType::Enum, K, AddressInfoList>;
    
    template<AddressType::Enum type>
    struct DedupAddressTypeFunctor {
        static constexpr DedupAddressType::Enum f() {
            return AddressInfo<type>::dedupType;
        }
    };
    
    static constexpr auto dedupTypeTable = blocksci::make_static_table<AddressType, DedupAddressTypeFunctor>();
    
    constexpr void addressTypeCheckThrow(size_t index) {  
        index >= AddressType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }

    constexpr DedupAddressType::Enum dedupType(AddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        addressTypeCheckThrow(index);
        return dedupTypeTable[index];
    }
    
    std::string addressName(AddressType::Enum type);
    
    constexpr bool isSpendable(AddressType::Enum t) {
        return isSpendable(dedupType(t));
    }
}

#endif /* address_info_hpp */
