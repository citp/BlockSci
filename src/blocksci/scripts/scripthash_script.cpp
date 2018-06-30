//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "scripthash_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "script_variant.hpp"
#include "bitcoin_base58.hpp"

namespace blocksci {
    using namespace script;
    
    ScriptHash::ScriptAddress(uint32_t scriptNum_, const ScriptHashData *rawData, const ScriptAccess &access) : BaseScript(scriptNum_, scriptType, *rawData, access), wrappedAddress(rawData->wrappedAddress), address(rawData->address) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(addressNum, access.getScriptData<scriptType>(addressNum), access) {}
    
    std::string ScriptHash::addressString() const {
        return CBitcoinAddress(address, AddressType::Enum::SCRIPTHASH, access->config).ToString();
    }
    
    ranges::optional<Address> ScriptHash::getWrappedAddress() const {
        if (wrappedAddress.scriptNum != 0) {
            return wrappedAddress;
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<AnyScript> ScriptHash::wrappedScript() const {
        auto add = getWrappedAddress();
        if (add) {
            return add->getScript(*access);
        }
        return ranges::nullopt;
    }
    
    std::string ScriptHash::toString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString();
        ss << ", wrappedAddress=";
        auto wrapped = wrappedScript();
        if (wrapped) {
            ss << wrapped->toPrettyString();
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
}
