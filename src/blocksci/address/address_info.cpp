//
//  script_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_info.hpp"

#include <sstream>

namespace blocksci {
    
    
    constexpr char AddressInfo<AddressType::PUBKEY>::name[];
    constexpr char AddressInfo<AddressType::PUBKEYHASH>::name[];
    constexpr char AddressInfo<AddressType::MULTISIG_PUBKEY>::name[];
    constexpr char AddressInfo<AddressType::WITNESS_PUBKEYHASH>::name[];
    constexpr char AddressInfo<AddressType::SCRIPTHASH>::name[];
    constexpr char AddressInfo<AddressType::WITNESS_SCRIPTHASH>::name[];
    constexpr char AddressInfo<AddressType::MULTISIG>::name[];
    constexpr char AddressInfo<AddressType::NULL_DATA>::name[];
    constexpr char AddressInfo<AddressType::NONSTANDARD>::name[];
    
    constexpr std::array<AddressType::Enum, 4> EquivAddressInfo<EquivAddressType::PUBKEY>::equivTypes;
    constexpr std::array<AddressType::Enum, 2> EquivAddressInfo<EquivAddressType::SCRIPTHASH>::equivTypes;
    constexpr std::array<AddressType::Enum, 1> EquivAddressInfo<EquivAddressType::MULTISIG>::equivTypes;
    constexpr std::array<AddressType::Enum, 1> EquivAddressInfo<EquivAddressType::NULL_DATA>::equivTypes;
    constexpr std::array<AddressType::Enum, 1> EquivAddressInfo<EquivAddressType::NONSTANDARD>::equivTypes;
    
    template<AddressType::Enum type>
    struct AddressNameFunctor {
        static std::string f() {
            return AddressInfo<type>::name;
        }
    };
    
    std::string addressName(AddressType::Enum type) {
        static auto table = make_static_table<AddressType, AddressNameFunctor>();
        auto index = static_cast<size_t>(type);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
}
