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
#include "scripts.hpp"
#include "script_variant.hpp"
#include "bitcoin_base58.hpp"
#include "chain/transaction.hpp"

namespace blocksci {
    using namespace script;
    
    ScriptHash::ScriptAddress(uint32_t scriptNum_, const ScriptHashData *rawData) : BaseScript(scriptNum_, scriptType, *rawData), wrappedAddress(rawData->wrappedAddress), address(rawData->address) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(addressNum, access.getScriptData<scriptType>(addressNum)) {}
    
    std::string ScriptHash::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(address, AddressType::Enum::SCRIPTHASH, config).ToString();
    }
    
    ranges::optional<Address> ScriptHash::getWrappedAddress() const {
        if (wrappedAddress.scriptNum != 0) {
            return wrappedAddress;
        } else {
            return ranges::nullopt;
        }
    }
    
    AnyScript ScriptHash::wrappedScript(const ScriptAccess &access) const {
        return wrappedAddress.getScript(access);
    }
    
    ranges::optional<Transaction> ScriptHash::transactionRevealed(const ChainAccess &chain) const {
        if (txRevealed != 0) {
            return Transaction(chain, txRevealed);
        } else {
            return ranges::nullopt;
        }
    }
    
    std::string ScriptHash::toString(const DataConfiguration &config) const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        auto wrapped = wrappedScript(access);
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ", wrappedAddress=";
        if (wrappedAddress.scriptNum > 0) {
            wrapped.toPrettyString(config, access);
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
}
