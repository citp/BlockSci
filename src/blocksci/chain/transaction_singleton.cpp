//
//  transaction_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "transaction.hpp"
#include "data_access.hpp"
#include "output.hpp"
#include "input.hpp"
#include "block.hpp"
#include "bitcoin_uint256.hpp"

namespace blocksci {
    
    ranges::optional<Transaction> Transaction::txWithHash(uint256 hash) {
        return txWithHash(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> Transaction::txWithHash(std::string hash) {
        return txWithHash(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain);
    }
    
    Transaction::Transaction(uint32_t index) : Transaction(index, *DataAccess::Instance().chain) {}
    
    Transaction::Transaction(uint32_t index, uint32_t height) : Transaction(index, height, *DataAccess::Instance().chain) {}
    
    ranges::optional<Output> getChangeOutput(const Transaction &tx) {
        return getChangeOutput(tx, *DataAccess::Instance().scripts);
    }
    
    bool isChangeOverTx(const Transaction &tx) {
        return isChangeOverTx(tx, *DataAccess::Instance().scripts);
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        return containsKeysetChange(tx, *DataAccess::Instance().scripts);
    }
}
