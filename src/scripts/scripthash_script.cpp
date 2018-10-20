//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.hpp"

#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/hash.hpp>
#include <internal/script_access.hpp>

#include <sstream>

namespace blocksci {
    using script::ScriptHash;
    using script::WitnessScriptHash;
    
    
    ranges::optional<AnyScript> ScriptHashBase::wrappedScript() const {
        auto add = getWrappedAddress();
        if (add) {
            return add->getScript();
        }
        return ranges::nullopt;
    }
    
    uint160 ScriptHashBase::getUint160Address() const {
        auto backing = getBackingData();
        if (backing->isSegwit) {
            return ripemd160(reinterpret_cast<const char *>(&backing->hash256), sizeof(backing->hash256));
        } else {
            return backing->hash160;
        }
    }
    
    ScriptHash::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptHash::addressString() const {
        return CBitcoinAddress(getAddressHash(), AddressType::Enum::SCRIPTHASH, getAccess().config.chainConfig).ToString();
    }
    
    std::string ScriptHash::toString() const {
        std::stringstream ss;
        ss << "ScriptHashAddress(" << addressString()<< ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "ScriptHashAddress(" << addressString();
        ss << ", wrapped_address=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
    
    WitnessScriptHash::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string WitnessScriptHash::addressString() const {
        std::vector<uint8_t> witprog;
        auto addressHash = getAddressHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&addressHash), reinterpret_cast<const uint8_t *>(&addressHash) + sizeof(addressHash));
        return segwit_addr::encode(getAccess().config.chainConfig, 0, witprog);
    }
    
    std::string WitnessScriptHash::toString() const {
        std::stringstream ss;
        ss << "WitnessScriptHashAddress(" << addressString() << ")";
        return ss.str();
    }
    
    std::string WitnessScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "WitnessScriptHashAddress(" << addressString();
        ss << ", wrapped_address=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
} // namespace blocksci
