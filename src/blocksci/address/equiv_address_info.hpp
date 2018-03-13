//
//  equiv_address_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#ifndef equiv_address_info_hpp
#define equiv_address_info_hpp

#include "address_fwd.hpp"
#include <blocksci/util/util.hpp>

namespace blocksci {
    template <>
    struct EquivAddressInfo<EquivAddressType::PUBKEY> {
        static constexpr char name[] = "pubkey_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 3> addressTypes = {{AddressType::PUBKEY, AddressType::PUBKEYHASH, AddressType::WITNESS_PUBKEYHASH}};
    };
    
    template <>
    struct EquivAddressInfo<EquivAddressType::SCRIPTHASH> {
        static constexpr char name[] = "scripthash_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 2> addressTypes = {{AddressType::SCRIPTHASH, AddressType::WITNESS_SCRIPTHASH}};
    };
    
    template <>
    struct EquivAddressInfo<EquivAddressType::MULTISIG> {
        static constexpr char name[] = "multisig_script";
        static constexpr bool equived = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::MULTISIG}};
    };
    
    template <>
    struct EquivAddressInfo<EquivAddressType::NONSTANDARD> {
        static constexpr char name[] = "nonstandard_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NONSTANDARD}};
    };
    
    template <>
    struct EquivAddressInfo<EquivAddressType::NULL_DATA> {
        static constexpr char name[] = "null_data_script";
        static constexpr bool equived = false;
        static constexpr bool spendable = false;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NULL_DATA}};
    };
    
    using EquivAddressInfoList = array_to_tuple_t<EquivAddressType::Enum, EquivAddressType::size, EquivAddressType::all>;
    
    template <template<EquivAddressType::Enum> class K>
    using to_equiv_address_tuple_t = apply_template_t<EquivAddressType::Enum, K, EquivAddressInfoList>;
    
    template<EquivAddressType::Enum type>
    struct SpendableFunctor {
        static constexpr bool f() {
            return EquivAddressInfo<type>::spendable;
        }
    };
    
    constexpr void scriptTypeCheckThrow(size_t index) {
        index >= EquivAddressType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }
    
    static constexpr auto spendableTable = blocksci::make_static_table<EquivAddressType, SpendableFunctor>();
    
    constexpr bool isSpendable(EquivAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return spendableTable[index];
    }
    
    template<EquivAddressType::Enum type>
    struct EquivedFunctor {
        static constexpr bool f() {
            return EquivAddressInfo<type>::equived;
        }
    };
    
    static constexpr auto equivedTable = blocksci::make_static_table<EquivAddressType, EquivedFunctor>();
    
    constexpr bool isEquived(EquivAddressType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return equivedTable[index];
    }
    
    std::string equivAddressName(EquivAddressType::Enum type);
}

#endif /* equiv_address_info_hpp */
