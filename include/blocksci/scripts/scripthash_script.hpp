//
//  scripthash_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scripthash_script_hpp
#define scripthash_script_hpp

#include "script.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/hash_combine.hpp>

namespace blocksci {
    class BLOCKSCI_EXPORT ScriptHashBase : public ScriptBase {
        const ScriptHashData *getBackingData() const {
            return reinterpret_cast<const ScriptHashData *>(ScriptBase::getData());
        }
    protected:
        ScriptHashBase(uint32_t scriptNum_, AddressType::Enum type_, const ScriptHashData *rawData_, DataAccess &access_) : ScriptBase(scriptNum_, type_, access_, rawData_) {}
        
    public:
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            auto wrapped = getWrappedAddress();
            if (wrapped) {
                visitFunc(*wrapped);
            }
        }
        
        ranges::optional<Address> getWrappedAddress() const {
            auto wrapped = getBackingData()->wrappedAddress;
            if (wrapped.scriptNum != 0) {
                return Address(wrapped.scriptNum, wrapped.type, getAccess());
            } else {
                return ranges::nullopt;
            }
        }
        
        ranges::optional<AnyScript> wrappedScript() const;
        
        uint160 getUint160Address() const;
        
        uint256 getUint256Address() const {
            return getBackingData()->hash256;
        }
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::SCRIPTHASH> : public ScriptHashBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::SCRIPTHASH;
        
        ScriptAddress(uint32_t addressNum_, const ScriptHashData *data_, DataAccess &access_) : ScriptHashBase(addressNum_, addressType, data_, access_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        uint160 getAddressHash() const {
            return getUint160Address();
        }
        
        std::string addressString() const;
        
        std::string toString() const;
        
        std::string toPrettyString() const;
    };
    
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::WITNESS_SCRIPTHASH> : public ScriptHashBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_SCRIPTHASH;
        
        ScriptAddress(uint32_t addressNum_, const ScriptHashData *data_, DataAccess &access_) : ScriptHashBase(addressNum_, addressType, data_, access_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        uint256 getAddressHash() const {
            return getUint256Address();
        }
        
        std::string addressString() const;
        
        std::string toString() const;
        
        std::string toPrettyString() const;
    };
} // namespace blocksci

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::ScriptHashBase> {
        size_t operator()(const blocksci::ScriptHashBase &address) const {
            std::size_t seed = 8736521;
            blocksci::hash_combine(seed, static_cast<const blocksci::ScriptBase &>(address));
            return seed;
        }
    };
    
    template<> struct BLOCKSCI_EXPORT hash<blocksci::ScriptAddress<blocksci::AddressType::SCRIPTHASH>> {
        size_t operator()(const blocksci::ScriptAddress<blocksci::AddressType::SCRIPTHASH> &address) const {
            std::size_t seed = 6543246;
            blocksci::hash_combine(seed, static_cast<const blocksci::ScriptHashBase &>(address));
            return seed;
        }
    };
    
    template<> struct BLOCKSCI_EXPORT hash<blocksci::ScriptAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>> {
        size_t operator()(const blocksci::ScriptAddress<blocksci::AddressType::WITNESS_SCRIPTHASH> &address) const {
            std::size_t seed = 7653543;
            blocksci::hash_combine(seed, static_cast<const blocksci::ScriptHashBase &>(address));
            return seed;
        }
    };
} // namespace std

#endif /* scripthash_script_hpp */
