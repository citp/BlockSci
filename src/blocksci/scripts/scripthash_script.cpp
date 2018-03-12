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
#include "bitcoin_segwit_addr.hpp"

#include <blocksci/util/data_access.hpp>

namespace blocksci {
    using namespace script;
    
    ScriptHashBase::ScriptHashBase(uint32_t scriptNum_, AddressType::Enum type_, const ScriptHashData *rawData_, const DataAccess &access) : ScriptBase(scriptNum_, type_, access), rawData(rawData_) {}
    
    ranges::optional<Address> ScriptHashBase::getWrappedAddress() const {
        auto wrapped = rawData->wrappedAddress;
        if (wrapped.scriptNum != 0) {
            return Address(wrapped, getAccess());
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<AnyScript> ScriptHashBase::wrappedScript() const {
        auto add = getWrappedAddress();
        if (add) {
            return add->getScript();
        }
        return ranges::nullopt;
    }
    
    uint160 ScriptHashBase::getUint160Address() const {
        return rawData->getHash160();
    }
    
    uint256 ScriptHashBase::getUint256Address() const {
        return rawData->hash256;
    }
    
    ScriptHash::ScriptAddress(uint32_t addressNum, const DataAccess &access) : ScriptHashBase(addressNum, addressType, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string ScriptHash::addressString() const {
        return CBitcoinAddress(getAddressHash(), AddressType::Enum::SCRIPTHASH, getAccess().config).ToString();
    }
    
    std::string ScriptHash::toString() const {
        std::stringstream ss;
        ss << "ScriptHashAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "ScriptHashAddress(";
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
    
    WitnessScriptHash::ScriptAddress(uint32_t addressNum_, const DataAccess &access) : ScriptHashBase(addressNum_, addressType, access.scripts->getScriptData<addressType>(addressNum_), access) {}
    
    std::string WitnessScriptHash::addressString() const {
        std::vector<uint8_t> witprog;
        auto addressHash = getAddressHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&addressHash), reinterpret_cast<const uint8_t *>(&addressHash) + sizeof(addressHash));
        return segwit_addr::encode(getAccess().config, 0, witprog);
    }
    
    std::string WitnessScriptHash::toString() const {
        std::stringstream ss;
        ss << "WitnessScriptHashAddress(";
        ss << "address=" << addressString();
        ss << ")";
        return ss.str();
    }
    
    std::string WitnessScriptHash::toPrettyString() const {
        std::stringstream ss;
        ss << "WitnessScriptHashAddress(";
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
