//
//  raw_address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "raw_address_pointer.hpp"

#include "hash.hpp"
#include "data_configuration.hpp"

#include "scripts/script_data.hpp"
#include "scripts/bitcoin_base58.hpp"

#include "hash.hpp"

#include <range/v3/utility/optional.hpp>


namespace blocksci {
    
    RawAddress::RawAddress(uint160 hash_, AddressType::Enum type_) : hash(hash_), type(type_) {}
    
    RawAddress::RawAddress(const PubkeyData &data) : RawAddress(data.address, AddressType::Enum::PUBKEYHASH) {}
    
    RawAddress::RawAddress(const ScriptHashData &data) : RawAddress(data.address, AddressType::Enum::SCRIPTHASH) {
        
    }
    
    ranges::optional<RawAddress> RawAddress::create(const DataConfiguration &config, const std::string &addressString) {
        CBitcoinAddress address{addressString};
        auto ret = address.Get(config);
        if (ret.second != AddressType::Enum::NONSTANDARD) {
            return RawAddress{ret.first, ret.second};
        } else {
            return ranges::nullopt;
        }
    }
}

namespace std {
    size_t std::hash<blocksci::RawAddress>::operator()(const blocksci::RawAddress &b) const {
        std::size_t seed = 9765487;
        
        hash_combine(seed, b.hash);
        hash_combine(seed, b.type);
        return seed;
    }
}
