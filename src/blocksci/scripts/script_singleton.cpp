//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "script.hpp"
#include "util/data_access.hpp"
#include "script_variant.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"

namespace blocksci {
    
    AnyScript Script::getScript() const {
        return getScript(*DataAccess::Instance().scripts);
    }
    
    Transaction BaseScript::getFirstTransaction() const {
        return getFirstTransaction(*DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> BaseScript::getTransactionRevealed() const {
        return getTransactionRevealed(*DataAccess::Instance().chain);
    }
    
    std::vector<Output> Script::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Input> Script::getInputs() const {
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
