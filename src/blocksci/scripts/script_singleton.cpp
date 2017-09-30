//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "script.hpp"
#include "data_access.hpp"

#include "chain/transaction.hpp"

namespace blocksci {
    
    std::unique_ptr<Script> Script::create(const ScriptPointer &pointer) {
        return create(*DataAccess::Instance().scripts, pointer);
    }
    
    std::unique_ptr<Script> Script::create(const Address &address) {
        return create(*DataAccess::Instance().scripts, address);
    }
    
    std::string Script::toPrettyString() const {
        auto &instance = DataAccess::Instance();
        return toPrettyString(instance.config, *instance.scripts);
    }
    
    std::string Script::toString() const {
        return toString(DataAccess::Instance().config);
    }
    
    std::vector<const Output *> Script::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<const Input *> Script::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Script::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Script::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Script::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(*instance.addressIndex, *instance.chain);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Script &script) {
    os << script.toString();
    return os;
}
