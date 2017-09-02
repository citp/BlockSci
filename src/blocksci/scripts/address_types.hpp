//
//  address_types.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_types_hpp
#define address_types_hpp

#include "address_pointer.hpp"
#include "address.hpp"
#include "script.hpp"
#include "pubkey.hpp"
#include <blocksci/uint256.hpp>

#include <stdio.h>

namespace blocksci {
    
    struct PubkeyData;
    struct PubkeyHashData;
    struct MultisigData;
    struct ScriptHashData;
    struct RawData;
    struct NonstandardScriptData;
    class ScriptAccess;
    struct DataConfiguration;
    
    template <ScriptType::Enum ScriptType>
    class ScriptAddress;
    
    namespace address {
        using Pubkey = ScriptAddress<ScriptType::Enum::PUBKEY>;
        using PubkeyHash = ScriptAddress<ScriptType::Enum::PUBKEYHASH>;
        using ScriptHash = ScriptAddress<ScriptType::Enum::SCRIPTHASH>;
        using Multisig = ScriptAddress<ScriptType::Enum::MULTISIG>;
        using OpReturn = ScriptAddress<ScriptType::Enum::NULL_DATA>;
        using Nonstandard = ScriptAddress<ScriptType::Enum::NONSTANDARD>;
    }
    
    template <>
    class ScriptAddress<ScriptType::Enum::PUBKEY> : public Address {
        
    public:
        CPubKey pubkey;
        
        ScriptAddress<ScriptType::Enum::PUBKEY>(const PubkeyData *rawData);
        ScriptAddress<ScriptType::Enum::PUBKEY>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Address &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::PUBKEY>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
    
    template <>
    class ScriptAddress<ScriptType::Enum::PUBKEYHASH> : public Address {
        
    public:
        uint160 address;
        
        ScriptAddress<ScriptType::Enum::PUBKEYHASH>(const uint160 &address);
        ScriptAddress<ScriptType::Enum::PUBKEYHASH>(const PubkeyHashData *rawData);
        ScriptAddress<ScriptType::Enum::PUBKEYHASH>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Address &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::PUBKEYHASH>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
    
    struct RawPubkeyHash {
        uint160 address;
        
        explicit RawPubkeyHash(uint160 address_) : address(address_) {}
        
        bool operator==(const RawPubkeyHash &other) const {
            return address == other.address;
        }
    };
    
    template <>
    class ScriptAddress<ScriptType::Enum::MULTISIG> : public Address {
        
    public:
        uint8_t required;
        uint8_t total;
        std::vector<AddressPointer> addresses;
        
        ScriptAddress<ScriptType::Enum::MULTISIG>(const MultisigData *rawData);
        ScriptAddress<ScriptType::Enum::MULTISIG>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Address &other) override;
        
        std::vector<AddressPointer> nestedAddressPointers() const override {
            return addresses;
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::MULTISIG>(uint32_t addressNum);
        #endif
    };
    
    template <>
    class ScriptAddress<ScriptType::Enum::SCRIPTHASH> : public Address {
        
    public:
        uint160 address;
        AddressPointer wrappedAddressPointer;
        
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(const ScriptHashData *rawData);
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(const uint160 &address);
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        
        bool operator==(const Address &other) override;
        
        std::vector<AddressPointer> nestedAddressPointers() const override {
            std::vector<AddressPointer> addresses;
            if (wrappedAddressPointer.addressNum != 0) {
                addresses.push_back(wrappedAddressPointer);
            }
            return addresses;
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
    
    template <>
    class ScriptAddress<ScriptType::Enum::NULL_DATA> : public Address {
        
    public:
        ScriptAddress<ScriptType::Enum::NULL_DATA>(const RawData *rawData);
        ScriptAddress<ScriptType::Enum::NULL_DATA>(const ScriptAccess &access, uint32_t addressNum);
        std::string data;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Address &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::NULL_DATA>(uint32_t addressNum);
        #endif
    };
    
    template <>
    class ScriptAddress<ScriptType::Enum::NONSTANDARD> : public Address {
        CScript inputScript;
        CScript outputScript;
        
    public:
        ScriptAddress<ScriptType::Enum::NONSTANDARD>(std::tuple<const NonstandardScriptData *, const NonstandardScriptData *> &&rawData);
        ScriptAddress<ScriptType::Enum::NONSTANDARD>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string inputString() const;
        std::string outputString() const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Address &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::NONSTANDARD>(uint32_t addressNum);
        #endif
    };
}

#endif /* address_types_hpp */
