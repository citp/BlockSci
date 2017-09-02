//
//  address_types.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_types.hpp"
#include "script_access.hpp"
#include "data_configuration.hpp"
#include "script_data.hpp"
#include "raw_address_pointer.hpp"
#include "hash.hpp"
#include "base58.hpp"

#include <sstream>


namespace blocksci {
    
    using namespace address;
    
    std::string pubkeyHashToAddressString(const DataConfiguration &config, const uint160 &addressHash) {
        return CBitcoinAddress(addressHash, ScriptType::Enum::PUBKEYHASH, config).ToString();
    }
    
    Pubkey::ScriptAddress(const PubkeyData *rawData) : pubkey(rawData->pubkey) {}
    
    Pubkey::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Pubkey(access.getScriptData<ScriptType::Enum::PUBKEY>(addressNum)) {}
    
    bool Pubkey::operator==(const Address &other) {
        auto otherA = dynamic_cast<const Pubkey *>(&other);
        return otherA && otherA->pubkey == pubkey;
    }
    
    std::string Pubkey::addressString(const DataConfiguration &config) const {
        return pubkeyHashToAddressString(config, pubkey.GetID());
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
    
    PubkeyHash::ScriptAddress(const uint160 &address_) : address(address_) {}
    
    PubkeyHash::ScriptAddress(const PubkeyHashData *rawData) : address(rawData->address) {}
    
    PubkeyHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : PubkeyHash(access.getScriptData<ScriptType::Enum::PUBKEYHASH>(addressNum)) {}
    
    bool PubkeyHash::operator==(const Address &other) {
        auto pubkeyOther = dynamic_cast<const PubkeyHash *>(&other);
        return pubkeyOther && pubkeyOther->address == address;
    }
    
    std::string PubkeyHash::addressString(const DataConfiguration &config) const {
        return pubkeyHashToAddressString(config, address);
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
    
    ScriptHash::ScriptAddress(const ScriptHashData *rawData) : address(rawData->address), wrappedAddressPointer(rawData->wrappedAddress) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(access.getScriptData<ScriptType::Enum::SCRIPTHASH>(addressNum)) {}
    
    ScriptHash::ScriptAddress(const uint160 &address_) : address(address_), wrappedAddressPointer() {}
    
    bool ScriptHash::operator==(const Address &other) {
        auto otherA = dynamic_cast<const ScriptHash *>(&other);
        return otherA && otherA->address == address;
    }
    
    std::string ScriptHash::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(address, ScriptType::Enum::SCRIPTHASH, config).ToString();
    }
    
    std::string ScriptHash::toString(const DataConfiguration &config) const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        auto wrappedAddress = wrappedAddressPointer.getAddress(access);
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ", wrappedAddress=";
        if (wrappedAddressPointer.addressNum > 0) {
            ss << wrappedAddress->toPrettyString(config, access);
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
    
    Multisig::ScriptAddress(const MultisigData *rawData) : required(rawData->m), total(rawData->n), addresses(rawData->getAddresses()) {}
    
    Multisig::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Multisig(access.getScriptData<ScriptType::Enum::MULTISIG>(addressNum)) {}
    
    bool Multisig::operator==(const Address &other) {
        auto multisigOther = dynamic_cast<const Multisig *>(&other);
        
        if (!multisigOther) {
            return false;
        }
        
        if (multisigOther->required != required || multisigOther->addresses.size() != addresses.size()) {
            return false;
        }
        
        for (size_t i = 0; i < addresses.size(); i++) {
            if (addresses[i] != multisigOther->addresses[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    std::string Multisig::toString(const DataConfiguration &) const {
        std::stringstream ss;
        ss << "MultisigAddress(required=" << static_cast<int>(required) << ", n=" << static_cast<int>(addresses.size()) << ", address_nums=[";
        uint32_t i = 0;
        for (auto &address : addresses) {
            ss << address.addressNum;
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "])";
        return ss.str();
    }
    
    std::string Multisig::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        std::stringstream ss;
        ss << static_cast<int>(required) << " of " << static_cast<int>(addresses.size()) << " multisig with addresses : [";
        uint32_t i = 0;
        for (auto &address : addresses) {
            ss << address.getAddress(access)->toPrettyString(config, access);
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "]";
        return ss.str();
    }
    
    OpReturn::ScriptAddress(const RawData *raw) : data(raw->getData()) {}
    
    OpReturn::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : OpReturn(access.getScriptData<ScriptType::Enum::NULL_DATA>(addressNum)) {}
    
    bool OpReturn::operator==(const Address &other) {
        auto otherA = dynamic_cast<const OpReturn *>(&other);
        return otherA && otherA->data == data;
    }
    
    std::string OpReturn::toString(const DataConfiguration &) const {
        std::stringstream ss;
        ss << "NulldataAddressData()";
        return ss.str();
    }
    
    std::string OpReturn::toPrettyString(const DataConfiguration &config, const ScriptAccess &) const {
        return toString(config);
    }
    
    Nonstandard::ScriptAddress(std::tuple<const NonstandardScriptData *, const NonstandardScriptData *> &&rawData) : inputScript(std::get<1>(rawData)->getScript()), outputScript(std::get<0>(rawData)->getScript()) {}
    
    Nonstandard::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Nonstandard(access.getScriptData<ScriptType::Enum::NONSTANDARD>(addressNum)) {}
    
    bool Nonstandard::operator==(const Address &other) {
        auto otherA = dynamic_cast<const Nonstandard *>(&other);
        return otherA && otherA->inputString() == inputString() && otherA->outputString() == outputString();
    }
    
    std::string Nonstandard::inputString() const {
        return ScriptToAsmStr(inputScript);
    }
    
    std::string Nonstandard::outputString() const {
        return ScriptToAsmStr(outputScript);
    }
    
    std::string Nonstandard::toString(const DataConfiguration &) const {
        std::stringstream ss;
        ss << "NonStandardScript()";
        return ss.str();
    }
    
    std::string Nonstandard::toPrettyString(const DataConfiguration &config, const ScriptAccess &) const {
        return toString(config);
    }
    
}
