//
//  dedup_address_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#ifndef dedup_address_info_hpp
#define dedup_address_info_hpp

#include "core_fwd.hpp"
#include "dedup_address_type.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/meta.hpp>

#include <vector>

namespace blocksci {
    template <>
    struct BLOCKSCI_EXPORT DedupAddressInfo<DedupAddressType::PUBKEY> {
        static constexpr char name[] = "pubkey_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 4> addressTypes = {{AddressType::PUBKEY, AddressType::PUBKEYHASH, AddressType::MULTISIG_PUBKEY, AddressType::WITNESS_PUBKEYHASH}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::PUBKEY;
        static constexpr AddressType::Enum reprType = AddressType::PUBKEYHASH;
    };
    
    template <>
    struct BLOCKSCI_EXPORT DedupAddressInfo<DedupAddressType::SCRIPTHASH> {
        static constexpr char name[] = "scripthash_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 2> addressTypes = {{AddressType::SCRIPTHASH, AddressType::WITNESS_SCRIPTHASH}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::SCRIPTHASH;
        static constexpr AddressType::Enum reprType = AddressType::SCRIPTHASH;
    };
    
    template <>
    struct BLOCKSCI_EXPORT DedupAddressInfo<DedupAddressType::MULTISIG> {
        static constexpr char name[] = "multisig_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::MULTISIG}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::MULTISIG;
        static constexpr AddressType::Enum reprType = AddressType::MULTISIG;
    };
    
    template <>
    struct BLOCKSCI_EXPORT DedupAddressInfo<DedupAddressType::NONSTANDARD> {
        static constexpr char name[] = "nonstandard_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NONSTANDARD}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::NONSTANDARD;
        static constexpr AddressType::Enum reprType = AddressType::NONSTANDARD;
    };
    
    template <>
    struct BLOCKSCI_EXPORT DedupAddressInfo<DedupAddressType::NULL_DATA> {
        static constexpr char name[] = "null_data_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = false;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NULL_DATA}};
        static constexpr EquivAddressType::Enum equivType = EquivAddressType::NULL_DATA;
        static constexpr AddressType::Enum reprType = AddressType::NULL_DATA;
    };
    
    template<DedupAddressType::Enum type>
    struct SpendableFunctor {
        static constexpr bool f() {
            return DedupAddressInfo<type>::spendable;
        }
    };
    
    constexpr void dedupTypeCheckThrow(size_t index) {
        index >= DedupAddressType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }
    
    static constexpr auto spendableTable = blocksci::make_static_table<DedupAddressType, SpendableFunctor>();
    
    constexpr bool BLOCKSCI_EXPORT isSpendable(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        dedupTypeCheckThrow(index);
        return spendableTable[index];
    }
    
    template<DedupAddressType::Enum type>
    struct EquivedFunctor {
        static constexpr bool f() {
            return DedupAddressInfo<type>::equived;
        }
    };
    
    static constexpr auto equivedTable = blocksci::make_static_table<DedupAddressType, EquivedFunctor>();
    
    constexpr bool BLOCKSCI_EXPORT isEquived(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        dedupTypeCheckThrow(index);
        return equivedTable[index];
    }
    
    template<DedupAddressType::Enum type>
    struct DedupEquivTypeFunctor {
        static constexpr EquivAddressType::Enum f() {
            return DedupAddressInfo<type>::equivType;
        }
    };
    
    static constexpr auto dedupEquivTable = blocksci::make_static_table<DedupAddressType, DedupEquivTypeFunctor>();
    
    constexpr EquivAddressType::Enum BLOCKSCI_EXPORT equivType(DedupAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        dedupTypeCheckThrow(index);
        return dedupEquivTable[index];
    }
    
    std::string BLOCKSCI_EXPORT dedupAddressName(DedupAddressType::Enum type);
    
    template<DedupAddressType::Enum type>
    struct DedupAddressTypesFunctor {
        static std::vector<AddressType::Enum> f() {
            auto &types = DedupAddressInfo<type>::addressTypes;
            return std::vector<AddressType::Enum>{types.begin(), types.end()};
        }
    };
    
    inline std::vector<AddressType::Enum> addressTypes(DedupAddressType::Enum t) {
        static auto &dedupAddressTypesTable = *[]() {
            auto nameTable = make_static_table<DedupAddressType, DedupAddressTypesFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(t);
        dedupTypeCheckThrow(index);
        return dedupAddressTypesTable[index];
    }
}

#endif /* dedup_address_info_hpp */
