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
        return getScript(*DataAccess::Instance().scripts);
    }
    
    uint32_t Address::getFirstTransactionIndex() const {
        return getFirstTransactionIndex(*DataAccess::Instance().scriptFirstSeen);
    }
    
    Transaction Address::getFirstTransaction() const {
        auto &instance = DataAccess::Instance();
        return getFirstTransaction(*instance.chain, *instance.scriptFirstSeen);
    }
    
    std::vector<const Output *> Address::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<const Input *> Address::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    size_t addressCount() {
        return addressCount(*DataAccess::Instance().scripts);
    }
    
}
