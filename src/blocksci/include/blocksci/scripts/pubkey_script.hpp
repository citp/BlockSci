//
//  pubkey_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef pubkey_script_hpp
#define pubkey_script_hpp

#include <blocksci/blocksci_export.h>
#include "pubkey_base_script.hpp"

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::PUBKEY> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::PUBKEY;
        
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_), access_) {}
        
        std::string addressString() const;
        
        std::string toString() const {
            std::stringstream ss;
            ss << "PubkeyAddress(" << addressString() << ")";
            return ss.str();
        }
        
        std::string toPrettyString() const {
            return toString();
        }
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::PUBKEYHASH> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::PUBKEYHASH;
        
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_), access_) {}
        
        std::string addressString() const;
        
        std::string toString() const {
            std::stringstream ss;
            ss << "PubkeyHashAddress(" << addressString() << ")";
            return ss.str();
        }
        
        std::string toPrettyString() const {
            return toString();
        }
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::WITNESS_PUBKEYHASH> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_PUBKEYHASH;
        
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_), access_) {}
        
        std::string addressString() const;
        
        std::string toString() const {
            std::stringstream ss;
            ss << "WitnessPubkeyAddress(" << addressString() << ")";
            return ss.str();
        }
        
        std::string toPrettyString() const {
            return addressString();
        }
    };
} // namespace blocksci

#endif /* pubkey_script_hpp */
