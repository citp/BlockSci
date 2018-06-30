//
//  pubkey_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef pubkey_script_hpp
#define pubkey_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include "bitcoin_pubkey.hpp"

namespace blocksci {
    struct PubkeyData;
    
    template <>
    class ScriptAddress<AddressType::Enum::PUBKEY> : public Script {
        
    public:
        CPubKey pubkey;
        
        ScriptAddress<AddressType::Enum::PUBKEY>(const PubkeyData *rawData);
        ScriptAddress<AddressType::Enum::PUBKEY>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Script &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<AddressType::Enum::PUBKEY>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
}




#endif /* pubkey_script_hpp */
