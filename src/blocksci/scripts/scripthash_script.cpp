//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "scripthash_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "script_variant.hpp"
#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.h"

namespace blocksci {
    using namespace script;
    
    ScriptHashBase::ScriptHashBase(uint32_t scriptNum_, AddressType::Enum type_, const ScriptHashData *rawData, const ScriptAccess &access) : Script(scriptNum_, type_, *rawData, access), wrappedAddress(rawData->wrappedAddress) {}
    
    ranges::optional<Address> ScriptHashBase::getWrappedAddress() const {
        if (wrappedAddress.scriptNum != 0) {
            return wrappedAddress;
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<AnyScript> ScriptHashBase::wrappedScript() const {
        auto add = getWrappedAddress();
        if (add) {
            return add->getScript(*access);
        }
        return ranges::nullopt;
    }
    
    ScriptHash::ScriptAddress(uint32_t scriptNum_, const ScriptHashData *rawData, const ScriptAccess &access) : ScriptHashBase(scriptNum_, addressType, rawData, access), address(rawData->getHash160()) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(addressNum, access.getScriptData<addressType>(addressNum), access) {}
    
    std::string ScriptHash::addressString() const {
        return CBitcoinAddress(address, AddressType::Enum::SCRIPTHASH, access->config).ToString();
    }
    
    std::string ScriptHash::toString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ", wrappedAddress=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
    
    WitnessScriptHash::ScriptAddress(uint32_t scriptNum_, const ScriptHashData *rawData, const ScriptAccess &access) : ScriptHashBase(scriptNum_, addressType, rawData, access), address(rawData->hash256) {}
    
    WitnessScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : WitnessScriptHash(addressNum, access.getScriptData<addressType>(addressNum), access) {}
    
    std::string WitnessScriptHash::addressString() const {
        std::vector<uint8_t> witprog;
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&address), reinterpret_cast<const uint8_t *>(&address) + sizeof(address));
        return segwit_addr::encode(access->config, 0, witprog);
    }
    
    std::string WitnessScriptHash::toString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string WitnessScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ", wrappedAddress=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
}
