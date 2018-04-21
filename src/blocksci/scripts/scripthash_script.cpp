//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "scripthash_script.hpp"
#include "script_variant.hpp"
#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.hpp"

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
    
    std::string ScriptHash::addressString() const {
        return CBitcoinAddress(getAddressHash(), AddressType::Enum::SCRIPTHASH, getAccess().config).ToString();
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
    
    std::string WitnessScriptHash::addressString() const {
        std::vector<uint8_t> witprog;
        auto addressHash = getAddressHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&addressHash), reinterpret_cast<const uint8_t *>(&addressHash) + sizeof(addressHash));
        return segwit_addr::encode(getAccess().config, 0, witprog);
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
