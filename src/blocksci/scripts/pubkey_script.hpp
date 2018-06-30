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

    template <>
    class ScriptAddress<ScriptType::Enum::PUBKEY> : public BaseScript {
    private:
        CPubKey pubkey;
    public:
        uint160 pubkeyhash;
        
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::PUBKEY;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const PubkeyData *rawData, const ScriptAccess &access);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        ranges::optional<CPubKey> getPubkey() const;
    };
}




#endif /* pubkey_script_hpp */
