//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "scripthash_script.hpp"
#include "script_variant.hpp"

namespace blocksci {
    using script::ScriptHash;
    using script::WitnessScriptHash;
    
    ranges::optional<AnyScript> ScriptHashBase::wrappedScript() const {
        auto add = getWrappedAddress();
        if (add) {
            return add->getScript();
        }
        return ranges::nullopt;
    }
    
    std::string ScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "ScriptHashAddress(" << addressString();
        ss << ", wrapped_address=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
    
    std::string WitnessScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "WitnessScriptHashAddress(" << addressString();
        ss << ", wrapped_address=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
} // namespace blocksci
