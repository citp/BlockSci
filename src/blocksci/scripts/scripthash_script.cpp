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
#include "bitcoin_base58.hpp"
#include "chain/transaction.hpp"

#include <boost/optional/optional.hpp>

namespace blocksci {
    using namespace script;
    
    ScriptHash::ScriptAddress(uint32_t scriptNum_, const ScriptHashData *rawData) : Script(scriptNum_), wrappedAddress(rawData->wrappedAddress), txRevealed(rawData->txRevealed), address(rawData->address) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(addressNum, access.getScriptData<scriptType>(addressNum)) {}
    
    bool ScriptHash::operator==(const Script &other) {
        auto otherA = dynamic_cast<const ScriptHash *>(&other);
        return otherA && otherA->address == address;
    }
    
    std::string ScriptHash::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(address, AddressType::Enum::SCRIPTHASH, config).ToString();
    }
    
    boost::optional<Address> ScriptHash::getWrappedAddress() const {
        if (wrappedAddress.addressNum != 0) {
            return wrappedAddress;
        } else {
            return boost::none;
        }
    }
    
    boost::optional<Transaction> ScriptHash::transactionRevealed(const ChainAccess &chain) const {
        if (txRevealed != 0) {
            return Transaction::txWithIndex(chain, txRevealed);
        } else {
            return boost::none;
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
        if (wrappedAddress.addressNum > 0) {
            ss << wrapped->toPrettyString(config, access);
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
}
