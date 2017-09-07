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


namespace blocksci {
    
    RawAddress::RawAddress(uint160 hash_, ScriptType::Enum type_) : hash(hash_), type(type_) {}
    
    RawAddress::RawAddress(const PubkeyData &data) : RawAddress(data.address, ScriptType::Enum::PUBKEY) {}
    
    RawAddress::RawAddress(const ScriptHashData &data) : RawAddress(data.address, ScriptType::Enum::SCRIPTHASH) {
        
    }
    
    boost::optional<RawAddress> RawAddress::create(const DataConfiguration &config, const std::string &addressString) {
        CBitcoinAddress address{addressString};
        auto ret = address.Get(config);
        if (ret.second != ScriptType::Enum::NONSTANDARD) {
            return RawAddress{ret.first, ret.second};
        } else {
            return boost::none;
        }
    }
}
