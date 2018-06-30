//
//  pubkey_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "pubkey_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "bitcoin_base58.hpp"

namespace blocksci {
    using namespace script;
    
    
    Pubkey::ScriptAddress(const PubkeyData *rawData) : pubkey(rawData->pubkey) {}
    
    Pubkey::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Pubkey(access.getScriptData<AddressType::Enum::PUBKEY>(addressNum)) {}
    
    bool Pubkey::operator==(const Script &other) {
        auto otherA = dynamic_cast<const Pubkey *>(&other);
        return otherA && otherA->pubkey == pubkey;
    }
    
    std::string Pubkey::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(pubkey.GetID(), AddressType::Enum::PUBKEYHASH, config).ToString();
    }
    
    std::string Pubkey::toString(const DataConfiguration &config) const {
        std::stringstream ss;
        ss << "PubkeyAddress(";
        ss << "address=" << addressString(config);
        ss << ")";
        return ss.str();
    }
    
    std::string Pubkey::toPrettyString(const DataConfiguration &config, const ScriptAccess &) const {
        return addressString(config);
    }
}
