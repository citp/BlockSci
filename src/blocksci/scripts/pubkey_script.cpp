//
//  pubkey_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "pubkey_script.hpp"

#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.hpp"
#include "script_access.hpp"
#include "script_data.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    using script::Pubkey;
    using script::PubkeyHash;
    using script::MultisigPubkey;
    using script::WitnessPubkeyHash;
    using script::Multisig;
    
    Pubkey::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<addressType>(addressNum_), access_) {}
    
    std::string Pubkey::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config).ToString();
    }
    
    std::string Pubkey::toString() const {
        std::stringstream ss;
        ss << "PubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
    
    std::string Pubkey::toPrettyString() const {
        return toString();
    }
    
    PubkeyHash::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<addressType>(addressNum_), access_) {}
    
    std::string PubkeyHash::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config).ToString();
    }
    
    std::string PubkeyHash::toString() const {
        std::stringstream ss;
        ss << "PubkeyHashAddress(" << addressString() << ")";
        return ss.str();
    }
    
    std::string PubkeyHash::toPrettyString() const {
        return toString();
    }
    
    WitnessPubkeyHash::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<addressType>(addressNum_), access_) {}
    
    std::string WitnessPubkeyHash::addressString() const {
        std::vector<uint8_t> witprog;
        auto pubkeyhash = getPubkeyHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&pubkeyhash), reinterpret_cast<const uint8_t *>(&pubkeyhash) + sizeof(pubkeyhash));
        return segwit_addr::encode(getAccess().config, 0, witprog);
    }
    
    std::string WitnessPubkeyHash::toString() const {
        std::stringstream ss;
        ss << "WitnessPubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
    
    std::string WitnessPubkeyHash::toPrettyString() const {
        return addressString();
    }
} // namespace blocksci
