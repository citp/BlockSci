//
//  pubkeyhash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "pubkeyhash_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "bitcoin_base58.hpp"

namespace blocksci {
    using namespace script;
    
    PubkeyHash::ScriptAddress(const uint160 &address_) : address(address_) {}
    
    PubkeyHash::ScriptAddress(const PubkeyHashData *rawData) : address(rawData->address) {}
    
    PubkeyHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : PubkeyHash(access.getScriptData<AddressType::Enum::PUBKEYHASH>(addressNum)) {}
    
    bool PubkeyHash::operator==(const Script &other) {
        auto pubkeyOther = dynamic_cast<const PubkeyHash *>(&other);
        return pubkeyOther && pubkeyOther->address == address;
    }
    
    std::string PubkeyHash::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(address, AddressType::Enum::PUBKEYHASH, config).ToString();
    }
    
    std::string PubkeyHash::toString(const DataConfiguration &config) const {
        std::stringstream ss;
        ss << "PubkeyHashAddress(";
        ss << "address=" << addressString(config);
        ss << ")";
        return ss.str();
    }
    
    std::string PubkeyHash::toPrettyString(const DataConfiguration &config, const ScriptAccess &) const {
        return addressString(config);
    }
}
