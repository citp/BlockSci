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
    constexpr char AddressInfo<AddressType::Enum::PUBKEY>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::PUBKEYHASH>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::SCRIPTHASH>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::MULTISIG>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::NULL_DATA>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::NONSTANDARD>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::WITNESS_PUBKEYHASH>::typeName[];
    constexpr char AddressInfo<AddressType::Enum::WITNESS_SCRIPTHASH>::typeName[];
    
    template<AddressType::Enum type>
    struct TypeNameFunctor {
        static std::string f() {
            return AddressInfo<type>::typeName;
        }
    };
    
    std::string GetTxnOutputType(AddressType::Enum type) {
        static auto table = make_static_table<AddressType, TypeNameFunctor>();
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
}
