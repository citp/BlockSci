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

#include <boost/optional/optional_fwd.hpp>

namespace blocksci {
    struct PubkeyData;
    
    template <>
    class ScriptAddress<ScriptType::Enum::PUBKEY> : public Script {
    private:
        CPubKey pubkey;
    public:
        uint160 pubkeyhash;
        
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const PubkeyData *rawData);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;
        
        boost::optional<CPubKey> getPubkey() const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<scriptType>(uint32_t addressNum);
        std::string addressString() const;
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
    };
}




#endif /* pubkey_script_hpp */
