//
//  dedup_address_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#ifndef dedup_address_info_hpp
#define dedup_address_info_hpp

#include "address_fwd.hpp"
#include <blocksci/util/static_table.hpp>

namespace blocksci {
    template <>
    struct DedupAddressInfo<DedupAddressType::PUBKEY> {
        static constexpr char name[] = "pubkey_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 4> addressTypes = {{AddressType::PUBKEY, AddressType::PUBKEYHASH, AddressType::MULTISIG_PUBKEY, AddressType::WITNESS_PUBKEYHASH}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::PUBKEY;
    };
    
    template <>
    struct DedupAddressInfo<DedupAddressType::SCRIPTHASH> {
        static constexpr char name[] = "scripthash_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 2> addressTypes = {{AddressType::SCRIPTHASH, AddressType::WITNESS_SCRIPTHASH}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::SCRIPTHASH;
    };
    
    template <>
    struct DedupAddressInfo<DedupAddressType::MULTISIG> {
        static constexpr char name[] = "multisig_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::MULTISIG}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::MULTISIG;
    };
    
    template <>
    struct DedupAddressInfo<DedupAddressType::NONSTANDARD> {
        static constexpr char name[] = "nonstandard_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NONSTANDARD}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::NONSTANDARD;
    };
    
    template <>
    struct DedupAddressInfo<DedupAddressType::NULL_DATA> {
        static constexpr char name[] = "null_data_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = false;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NULL_DATA}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::NULL_DATA;
    };
    
    template<DedupAddressType::Enum type>
    struct SpendableFunctor {
        static constexpr bool f() {
            return DedupAddressInfo<type>::spendable;
        }
    };
    
    constexpr void scriptTypeCheckThrow(size_t index) {
        index >= DedupAddressType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }
    
    static constexpr auto spendableTable = blocksci::make_static_table<DedupAddressType, SpendableFunctor>();
    
    constexpr bool isSpendable(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return spendableTable[index];
    }
    
    template<DedupAddressType::Enum type>
    struct EquivedFunctor {
        static constexpr bool f() {
            return DedupAddressInfo<type>::equived;
        }
    };
    
    static constexpr auto equivedTable = blocksci::make_static_table<DedupAddressType, EquivedFunctor>();
    
    constexpr bool isEquived(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return equivedTable[index];
    }
    
    template<DedupAddressType::Enum type>
    struct DedupEquivTypeFunctor {
        static constexpr EquivAddressType::Enum f() {
            return DedupAddressInfo<type>::equivType;
        }
    };
    
    static constexpr auto dedupEquivTable = blocksci::make_static_table<DedupAddressType, DedupEquivTypeFunctor>();
    
    constexpr EquivAddressType::Enum equivType(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return dedupEquivTable[index];
    }
    
    std::string dedupAddressName(DedupAddressType::Enum type);
}

#endif /* dedup_address_info_hpp */
