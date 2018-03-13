//
//  equiv_address_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#include "equiv_address.hpp"
#include "util/data_access.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"
#include "scripts/script.hpp"
#include "scripts/script_variant.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    
    uint64_t EquivAddress::calculateBalance(BlockHeight height) const {
        auto &instance = DataAccess::Instance();
        return calculateBalance(height, *instance.addressIndex, *instance.chain);
    }
    
    std::vector<Output> EquivAddress::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Input> EquivAddress::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> EquivAddress::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> EquivAddress::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> EquivAddress::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(*instance.addressIndex, *instance.chain);
    }
}
