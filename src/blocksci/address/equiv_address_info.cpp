//
//  equiv_address_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#include "equiv_address_info.hpp"

namespace blocksci {
    constexpr char EquivAddressInfo<EquivAddressType::PUBKEY>::name[];
    constexpr char EquivAddressInfo<EquivAddressType::SCRIPTHASH>::name[];
    constexpr char EquivAddressInfo<EquivAddressType::MULTISIG>::name[];
    constexpr char EquivAddressInfo<EquivAddressType::NULL_DATA>::name[];
    constexpr char EquivAddressInfo<EquivAddressType::NONSTANDARD>::name[];
    
    template<EquivAddressType::Enum type>
    struct EquivAddressNameFunctor {
        static std::string f() {
            return EquivAddressInfo<type>::name;
        }
    };
    
    std::string equivAddressName(EquivAddressType::Enum type) {
        static auto table = make_static_table<EquivAddressType, EquivAddressNameFunctor>();
        auto index = static_cast<size_t>(type);
        if (index >= EquivAddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
}
