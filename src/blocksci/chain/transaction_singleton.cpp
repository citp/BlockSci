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
    uint256 Transaction::getHash() const {
        return getHash(*DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> Transaction::txWithHash(uint256 hash) {
        return txWithHash(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> Transaction::txWithHash(std::string hash) {
        return txWithHash(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain);
    }
    
    Transaction Transaction::txWithIndex(uint32_t index) {
        return txWithIndex(*DataAccess::Instance().chain, index);
    }
    
    Transaction Transaction::txWithIndex(uint32_t index, uint32_t height) {
        return txWithIndex(*DataAccess::Instance().chain, index, height);
    }
    
    const Output * getChangeOutput(const Transaction &tx) {
        return getChangeOutput(tx, *DataAccess::Instance().scripts);
    }
    
    bool isChangeOverTx(const Transaction &tx) {
        return isChangeOverTx(tx, *DataAccess::Instance().scripts);
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        return containsKeysetChange(tx, *DataAccess::Instance().scripts);
    }
}
