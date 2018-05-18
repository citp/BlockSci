//
//  script_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_info.hpp"

#include <iostream>

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
        static auto &table = *[]() {
            auto nameTable = make_static_table<AddressType, AddressNameFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(type);
        return table.at(index);
    }
    
    template<EquivAddressType::Enum type>
    struct EquivAddressTypesFunctor {
        static std::vector<AddressType::Enum> f() {
            auto types = EquivAddressInfo<type>::equivTypes;
            return std::vector<AddressType::Enum>{types.begin(), types.end()};
        }
    };
    
    std::vector<AddressType::Enum> equivAddressTypes(EquivAddressType::Enum t) {
        static auto &equivAddressTypesTable = *[]() {
            auto nameTable = make_static_table<EquivAddressType, EquivAddressTypesFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(t);
        addressTypeCheckThrow(index);
        return equivAddressTypesTable[index];
    }
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type) {
    os << addressName(type);
    return os;
}
