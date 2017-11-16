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

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    using namespace script;
    
    
    Pubkey::ScriptAddress(uint32_t scriptNum_, const PubkeyData *rawData, const ScriptAccess &access) : BaseScript(scriptNum_, scriptType, *rawData, access), pubkey(rawData->pubkey), pubkeyhash(rawData->address) {}
    
    Pubkey::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Pubkey(addressNum, access.getScriptData<scriptType>(addressNum), access) {}
    
    ranges::optional<CPubKey> Pubkey::getPubkey() const {
        if (pubkey.IsValid()) {
            return pubkey;
        } else {
            return ranges::nullopt;
        }
    }
    
    std::string Pubkey::addressString() const {
        return CBitcoinAddress(pubkeyhash, AddressType::Enum::PUBKEYHASH, access->config).ToString();
    }
    
    std::string Pubkey::toString() const {
        std::stringstream ss;
        ss << "PubkeyAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string Pubkey::toPrettyString() const {
        return addressString();
    }
}
