//
//  pubkeyhash_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef pubkeyhash_script_hpp
#define pubkeyhash_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include <blocksci/bitcoin_uint256.hpp>

namespace blocksci {
    struct PubkeyHashData;
    
    template <>
    class ScriptAddress<AddressType::Enum::PUBKEYHASH> : public Script {
        
    public:
        uint160 address;
        
        ScriptAddress<AddressType::Enum::PUBKEYHASH>(const uint160 &address);
        ScriptAddress<AddressType::Enum::PUBKEYHASH>(const PubkeyHashData *rawData);
        ScriptAddress<AddressType::Enum::PUBKEYHASH>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Script &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<AddressType::Enum::PUBKEYHASH>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
}

#endif /* pubkeyhash_script_hpp */
