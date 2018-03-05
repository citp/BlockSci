//
//  dedup_address_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#include "dedup_address.hpp"
#include "util/data_access.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"
#include "scripts/script.hpp"
#include "scripts/script_variant.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    
    uint64_t DedupAddress::calculateBalance(BlockHeight height) const {
        auto &instance = DataAccess::Instance();
        return calculateBalance(height, *instance.addressIndex, *instance.chain);
    }
    
    std::vector<Output> DedupAddress::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Input> DedupAddress::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> DedupAddress::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> DedupAddress::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> DedupAddress::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(*instance.addressIndex, *instance.chain);
    }
}
