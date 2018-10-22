//
//  pubkey_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef pubkey_script_hpp
#define pubkey_script_hpp

#include "pubkey_base_script.hpp"

#include <blocksci/blocksci_export.h>

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::PUBKEY> : public PubkeyAddressBase {
    
    public:
        constexpr static AddressType::Enum addressType = AddressType::PUBKEY;
        
        ScriptAddress(uint32_t addressNum_, const PubkeyData *data_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, data_, access_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string addressString() const;
        
        std::string toString() const;
        
        std::string toPrettyString() const {
            return toString();
        }
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::PUBKEYHASH> : public PubkeyAddressBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::PUBKEYHASH;
        
        ScriptAddress(uint32_t addressNum_, const PubkeyData *data_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, data_, access_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string addressString() const;
        
        std::string toString() const;
        
        std::string toPrettyString() const {
            return toString();
        }
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::WITNESS_PUBKEYHASH> : public PubkeyAddressBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_PUBKEYHASH;
        
        ScriptAddress(uint32_t addressNum_, const PubkeyData *data_, DataAccess &access_) : PubkeyAddressBase(addressNum_, addressType, data_, access_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string addressString() const;
        
        std::string toString() const;
        
        std::string toPrettyString() const {
            return toString();
        }
    };
} // namespace blocksci

#endif /* pubkey_script_hpp */
