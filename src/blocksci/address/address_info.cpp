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
    
    
    constexpr char AddressInfo<AddressType::Enum::PUBKEY>::name[];
    constexpr char AddressInfo<AddressType::Enum::PUBKEYHASH>::name[];
    constexpr char AddressInfo<AddressType::Enum::WITNESS_PUBKEYHASH>::name[];
    constexpr char AddressInfo<AddressType::Enum::SCRIPTHASH>::name[];
    constexpr char AddressInfo<AddressType::Enum::WITNESS_SCRIPTHASH>::name[];
    constexpr char AddressInfo<AddressType::Enum::MULTISIG>::name[];
    constexpr char AddressInfo<AddressType::Enum::NULL_DATA>::name[];
    constexpr char AddressInfo<AddressType::Enum::NONSTANDARD>::name[];
    
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
