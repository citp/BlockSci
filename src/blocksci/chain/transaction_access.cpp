//
//  transaction_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "transaction.hpp"
#include "data_access.hpp"
#include "output.hpp"
#include "input.hpp"

namespace blocksci {
    uint256 Transaction::getHash() const {
        return getHash(DataAccess::Instance().chain);
    }
    
    const Block &Transaction::block() const {
        return block(DataAccess::Instance().chain);
    }
    
    boost::optional<Transaction> Transaction::txWithHash(uint256 hash) {
        return txWithHash(DataAccess::Instance().chain, hash);
    }
    
    Transaction Transaction::txWithIndex(uint32_t index) {
        return txWithIndex(DataAccess::Instance().chain, index);
    }
    
    Transaction Transaction::txWithIndex(uint32_t index, uint32_t height) {
        return txWithIndex(DataAccess::Instance().chain, index, height);
    }
    
    boost::optional<Transaction> Transaction::txWithHash(std::string hash) {
        return txWithHash(DataAccess::Instance().chain, hash);
    }
    
    const Transaction &Transaction::create(uint32_t index) {
        return create(DataAccess::Instance().chain, index);
    }
    
    std::vector<Transaction> getTransactionsFromHashes(const std::vector<std::string> &txHashes) {
        return getTransactionsFromHashes(DataAccess::Instance().chain, txHashes);
    }
    
    const Output * getChangeOutput(const Transaction &tx) {
        auto &instance = DataAccess::Instance();
        return getChangeOutput(instance.scriptFirstSeen, tx);
    }
    
    bool isChangeOverTx(const Transaction &tx) {
        return isChangeOverTx(tx, DataAccess::Instance().scripts);
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        return containsKeysetChange(tx, DataAccess::Instance().scripts);
    }
}
