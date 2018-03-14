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
#include "scripts_fwd.hpp"
#include <blocksci/address/address.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    class ScriptHashBase : public ScriptBase<ScriptHashBase> {
        friend class ScriptBase<ScriptHashBase>;
        const ScriptHashData *rawData;
    protected:
        ScriptHashBase(uint32_t scriptNum, AddressType::Enum type, const ScriptHashData *rawData, const DataAccess &access);
        
    public:
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            auto wrapped = getWrappedAddress();
            if (wrapped) {
                visitFunc(*wrapped);
            }
        }
        
        ranges::optional<Address> getWrappedAddress() const;
        ranges::optional<AnyScript> wrappedScript() const;
        
        uint160 getUint160Address() const;
        uint256 getUint256Address() const;
    };
    
    template <>
    class ScriptAddress<AddressType::SCRIPTHASH> : public ScriptHashBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::SCRIPTHASH;
        
        ScriptAddress(uint32_t addressNum, const DataAccess &access);
        
        uint160 getAddressHash() const {
            return getUint160Address();
        }
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
    
    template <>
    class ScriptAddress<AddressType::WITNESS_SCRIPTHASH> : public ScriptHashBase {
    public:
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_SCRIPTHASH;
        
        ScriptAddress(uint32_t addressNum, const DataAccess &access);
        
        uint256 getAddressHash() const {
            return getUint256Address();
        }
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
}

#endif /* scripthash_script_hpp */
