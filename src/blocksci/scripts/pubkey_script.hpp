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
    class ScriptAddress<ScriptType::Enum::PUBKEY> : public Script {
    private:
        CPubKey pubkey;
    public:
        uint160 pubkeyhash;
        
        ScriptAddress<ScriptType::Enum::PUBKEY>(const PubkeyData *rawData);
        ScriptAddress<ScriptType::Enum::PUBKEY>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Script &other) override;
        
        boost::optional<CPubKey> getPubkey() const {
            if (pubkey.IsValid()) {
                return pubkey;
            } else {
                return boost::none;
            }
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::PUBKEY>(uint32_t addressNum);
        std::string addressString() const;
        #endif
    };
}




#endif /* pubkey_script_hpp */
