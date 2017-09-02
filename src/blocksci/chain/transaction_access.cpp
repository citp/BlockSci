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
    
    std::string getOpReturnData(const Transaction &tx) {
        return getOpReturnData(tx, DataAccess::Instance().scripts);
    }
    
    const Output * getChangeOutput(const Transaction &tx) {
        auto &instance = DataAccess::Instance();
        return getChangeOutput(instance.scriptsFirstSeen, tx);
    }
    
    bool isChangeOverTx(const Transaction &tx) {
        return isChangeOverTx(tx, DataAccess::Instance().scripts);
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        return containsKeysetChange(tx, DataAccess::Instance().scripts);
    }
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddresses(const Transaction &tx, int maxDepth, const Input &inputToReplace) {
        return getSourceAddresses(DataAccess::Instance().chain, tx, maxDepth, inputToReplace);
    }
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddressesList(const Transaction &tx, int maxDepth, const Input &inputToReplace, const std::vector<Transaction> &cjTxes) {
        return getSourceAddressesList(DataAccess::Instance().chain, tx, maxDepth, inputToReplace, cjTxes);
    }
}
