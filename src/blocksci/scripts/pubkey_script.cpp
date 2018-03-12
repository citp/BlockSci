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
#include "bitcoin_segwit_addr.hpp"
#include <blocksci/util/data_access.hpp>

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    using namespace script;
    
    PubkeyAddressBase::PubkeyAddressBase(uint32_t scriptNum_, AddressType::Enum type_, const PubkeyData *rawData, const DataAccess &access) : Script(scriptNum_, type_, *rawData, access), pubkey(rawData->pubkey), pubkeyhash(rawData->address) {}
    
    ranges::optional<CPubKey> PubkeyAddressBase::getPubkey() const {
        if (pubkey.IsValid()) {
            return pubkey;
        } else {
            return ranges::nullopt;
        }
    }
    
    Pubkey::ScriptAddress(uint32_t addressNum, const DataAccess &access) : PubkeyAddressBase(addressNum, addressType, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
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
    
    PubkeyHash::ScriptAddress(uint32_t addressNum, const DataAccess &access) : PubkeyAddressBase(addressNum, addressType, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string PubkeyHash::addressString() const {
        return CBitcoinAddress(pubkeyhash, AddressType::Enum::PUBKEYHASH, access->config).ToString();
    }
    
    std::string PubkeyHash::toString() const {
        std::stringstream ss;
        ss << "PubkeyHashAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string PubkeyHash::toPrettyString() const {
        return addressString();
    }
    
    MultisigPubkey::ScriptAddress(uint32_t addressNum, const DataAccess &access) : PubkeyAddressBase(addressNum, addressType, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string MultisigPubkey::addressString() const {
        return CBitcoinAddress(pubkeyhash, AddressType::Enum::MULTISIG_PUBKEY, access->config).ToString();
    }
    
    std::string MultisigPubkey::toString() const {
        std::stringstream ss;
        ss << "MultisigPubkeyAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string MultisigPubkey::toPrettyString() const {
        return addressString();
    }
    
    WitnessPubkeyHash::ScriptAddress(uint32_t addressNum, const DataAccess &access) : PubkeyAddressBase(addressNum, addressType, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string WitnessPubkeyHash::addressString() const {
        std::vector<uint8_t> witprog;
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&pubkeyhash), reinterpret_cast<const uint8_t *>(&pubkeyhash) + sizeof(pubkeyhash));
        return segwit_addr::encode(access->config, 0, witprog);
    }
    
    std::string WitnessPubkeyHash::toString() const {
        std::stringstream ss;
        ss << "WitnessPubkeyAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string WitnessPubkeyHash::toPrettyString() const {
        return addressString();
    }
}
