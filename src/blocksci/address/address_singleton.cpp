//
//  address_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "address.hpp"
#include "data_access.hpp"
#include "chain/transaction.hpp"
#include "scripts/script.hpp"

namespace blocksci {
    
    std::unique_ptr<Script> Address::getScript() const {
        return getScript(DataAccess::Instance().scripts);
    }
    
    uint32_t Address::getFirstTransactionIndex() const {
        return getFirstTransactionIndex(DataAccess::Instance().addressFirstSeen);
    }
    
    Transaction Address::getFirstTransaction() const {
        auto &instance = DataAccess::Instance();
        return getFirstTransaction(instance.chain, instance.addressFirstSeen);
    }
    
    std::vector<const Output *> Address::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(instance.addressIndex, instance.chain);
    }
    
    std::vector<const Input *> Address::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(instance.addressIndex, instance.chain);
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(instance.addressIndex, instance.chain);
    }
    
    std::vector<Transaction> Address::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(instance.addressIndex, instance.chain);
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(instance.addressIndex, instance.chain);
    }
    
    boost::optional<Address> getAddressFromString(const std::string &addressString) {
        auto &instance = DataAccess::Instance();
        return getAddressFromString(instance.config, instance.scripts, addressString);
    }
    
    std::vector<Address> getAddressesFromStrings(const std::vector<std::string> &addressStrings) {
        auto &instance = DataAccess::Instance();
        return getAddressesFromStrings(instance.config, instance.scripts, addressStrings);
    }
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix) {
        auto &instance = DataAccess::Instance();
        return getAddressesWithPrefix(instance.config, instance.scripts, prefix);
    }
    
    size_t addressCount() {
        return addressCount(DataAccess::Instance().scripts);
    }
    
}
