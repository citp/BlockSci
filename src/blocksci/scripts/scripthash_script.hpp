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
    class ScriptHashBase : public Script {
    private:
        Address wrappedAddress;
    public:
        ScriptHashBase(uint32_t scriptNum, AddressType::Enum type, const ScriptHashData *rawData, const ScriptAccess &access);
        
        ranges::optional<Address> getWrappedAddress() const;
        
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            if (hasBeenSpent()) {
                visitFunc(wrappedAddress);
            }
        }
        
        ranges::optional<AnyScript> wrappedScript() const;
    };
    
    template <>
    class ScriptAddress<AddressType::SCRIPTHASH> : public ScriptHashBase {
    public:
        uint160 address;
        
        constexpr static AddressType::Enum addressType = AddressType::SCRIPTHASH;
        
        ScriptAddress(uint32_t scriptNum, const ScriptHashData *rawData, const ScriptAccess &access);
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
    
    template <>
    class ScriptAddress<AddressType::WITNESS_SCRIPTHASH> : public ScriptHashBase {
    public:
        uint256 address;
        
        constexpr static AddressType::Enum addressType = AddressType::WITNESS_SCRIPTHASH;
        
        ScriptAddress(uint32_t scriptNum, const ScriptHashData *rawData, const ScriptAccess &access);
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
}

#endif /* scripthash_script_hpp */
