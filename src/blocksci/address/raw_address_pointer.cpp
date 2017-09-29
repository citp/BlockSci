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

#include <boost/optional/optional.hpp>


namespace blocksci {
    
    RawAddress::RawAddress(uint160 hash_, AddressType::Enum type_) : hash(hash_), type(type_) {}
    
    RawAddress::RawAddress(const PubkeyData &data) : RawAddress(data.address, AddressType::Enum::PUBKEYHASH) {}
    
    RawAddress::RawAddress(const ScriptHashData &data) : RawAddress(data.address, AddressType::Enum::SCRIPTHASH) {
        
    }
    
    boost::optional<RawAddress> RawAddress::create(const DataConfiguration &config, const std::string &addressString) {
        CBitcoinAddress address{addressString};
        auto ret = address.Get(config);
        if (ret.second != AddressType::Enum::NONSTANDARD) {
            return RawAddress{ret.first, ret.second};
        } else {
            return boost::none;
        }
    }
}
