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

#include <boost/optional/optional.hpp>

namespace blocksci {
    using namespace script;
    
    
    Pubkey::ScriptAddress(uint32_t scriptNum_, const PubkeyData *rawData) : Script(scriptNum_), pubkey(rawData->pubkey), pubkeyhash(rawData->address) {}
    
    Pubkey::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Pubkey(addressNum, access.getScriptData<scriptType>(addressNum)) {}
    
    bool Pubkey::operator==(const Script &other) {
        auto otherA = dynamic_cast<const Pubkey *>(&other);
        return otherA && otherA->pubkey == pubkey;
    }
    
    boost::optional<CPubKey> Pubkey::getPubkey() const {
        if (pubkey.IsValid()) {
            return pubkey;
        } else {
            return boost::none;
        }
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
