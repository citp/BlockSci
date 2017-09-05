//
//  scripthash_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scripthash_script_hpp
#define scripthash_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include <blocksci/bitcoin_uint256.hpp>

namespace blocksci {
    struct ScriptHashData;
    
    template <>
    class ScriptAddress<AddressType::Enum::SCRIPTHASH> : public Script {
        
    public:
        uint160 address;
        Address wrappedAddress;
        
        ScriptAddress<AddressType::Enum::SCRIPTHASH>(const ScriptHashData *rawData);
        ScriptAddress<AddressType::Enum::SCRIPTHASH>(const uint160 &address);
        ScriptAddress<AddressType::Enum::SCRIPTHASH>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        
        bool operator==(const Script &other) override;
        
        std::vector<Address> nestedAddresses() const override {
            std::vector<Address> addresses;
            if (wrappedAddress.addressNum != 0) {
                addresses.push_back(wrappedAddress);
            }
            return addresses;
        }
        
        std::unique_ptr<Script> wrappedScript(const ScriptAccess &access) const {
            return wrappedAddress.getScript(access);
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<AddressType::Enum::SCRIPTHASH>(uint32_t addressNum);
        std::string addressString() const;
        std::unique_ptr<Script> wrappedScript() const;
        #endif
    };
}

#endif /* scripthash_script_hpp */
