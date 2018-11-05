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
    constexpr char AddressInfo<AddressType::WITNESS_UNKNOWN>::name[];
    

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
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type) {
    os << addressName(type);
    return os;
}
