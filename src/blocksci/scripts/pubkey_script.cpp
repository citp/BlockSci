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
    
    PubkeyAddressBase::PubkeyAddressBase(uint32_t scriptNum_, AddressType::Enum type_, const PubkeyData *rawData_, const DataAccess &access_) : ScriptBase(scriptNum_, type_, access_), rawData(rawData_) {}
    
    ranges::optional<CPubKey> PubkeyAddressBase::getPubkey() const {
        if (rawData->pubkey.IsValid()) {
            return rawData->pubkey;
        } else {
            return ranges::nullopt;
        }
    }
    
    uint160 PubkeyAddressBase::getPubkeyHash() const {
        return rawData->address;
    }
    
    std::vector<Address> PubkeyAddressBase::getIncludingMultisigs() const {
        return getAccess().addressIndex->getIncludingMultisigs(*this);
    }
    
    Pubkey::ScriptAddress(uint32_t addressNum_, const DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts->getScriptData<addressType>(addressNum_), access_) {}
    
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
    
    PubkeyHash::ScriptAddress(uint32_t addressNum_, const DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts->getScriptData<addressType>(addressNum_), access_) {}
    
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
    
    MultisigPubkey::ScriptAddress(uint32_t addressNum_, const DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts->getScriptData<addressType>(addressNum_), access_) {}
    
    std::string MultisigPubkey::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::MULTISIG_PUBKEY, getAccess().config).ToString();
    }
    
    std::string MultisigPubkey::toString() const {
        std::stringstream ss;
        ss << "MultisigPubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
    
    std::string MultisigPubkey::toPrettyString() const {
        return addressString();
    }
    
    WitnessPubkeyHash::ScriptAddress(uint32_t addressNum_, const DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts->getScriptData<addressType>(addressNum_), access_) {}
    
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
