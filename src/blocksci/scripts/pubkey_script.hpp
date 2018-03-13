//
//  pubkey_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef pubkey_script_hpp
#define pubkey_script_hpp

#include "script.hpp"
#include "bitcoin_pubkey.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {

    class PubkeyAddressBase : public Script {
    private:
        CPubKey pubkey;
    public:
        uint160 pubkeyhash;
        
        PubkeyAddressBase(uint32_t scriptNum, AddressType::Enum type, const PubkeyData *rawData, const ScriptAccess &access);
        PubkeyAddressBase(const ScriptAccess &access, uint32_t addressNum, AddressType::Enum type);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        ranges::optional<CPubKey> getPubkey() const;
    };
    
    template <>
    class ScriptAddress<AddressType::PUBKEY> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::PUBKEY;
        
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
    
    template <>
    class ScriptAddress<AddressType::PUBKEYHASH> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::PUBKEYHASH;
        
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
    
    template <>
    class ScriptAddress<AddressType::WITNESS_PUBKEYHASH> : public PubkeyAddressBase {
    public:
        using PubkeyAddressBase::PubkeyAddressBase;
        
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_PUBKEYHASH;
        
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
}




#endif /* pubkey_script_hpp */
