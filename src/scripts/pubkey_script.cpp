//
//  pubkey_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <sstream>

namespace blocksci {
    
    ScriptAddress<AddressType::PUBKEY>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::PUBKEY>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config.chainConfig).ToString();
    }
    
    std::string ScriptAddress<AddressType::PUBKEY>::toString() const {
        std::stringstream ss;
        ss << "PubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
    
    ScriptAddress<AddressType::PUBKEYHASH>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::PUBKEYHASH>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config.chainConfig).ToString();
    }
    
    std::string ScriptAddress<AddressType::PUBKEYHASH>::toString() const {
        std::stringstream ss;
        ss << "PubkeyHashAddress(" << addressString() << ")";
        return ss.str();
    }
    
    ScriptAddress<AddressType::WITNESS_PUBKEYHASH>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::WITNESS_PUBKEYHASH>::addressString() const {
        std::vector<uint8_t> witprog;
        auto pubkeyhash = getPubkeyHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&pubkeyhash), reinterpret_cast<const uint8_t *>(&pubkeyhash) + sizeof(pubkeyhash));
        return segwit_addr::encode(getAccess().config.chainConfig, 0, witprog);
    }
    
    std::string ScriptAddress<AddressType::WITNESS_PUBKEYHASH>::toString() const {
        std::stringstream ss;
        ss << "WitnessPubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
} // namespace blocksci
