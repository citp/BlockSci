//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "script.hpp"
#include "data_access.hpp"
#include "scripts.hpp"

#include "chain/transaction.hpp"

#include <boost/variant.hpp>

namespace blocksci {
    
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
