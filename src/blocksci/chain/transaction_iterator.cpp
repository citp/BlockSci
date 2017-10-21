//
//  transaction_iterator.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/20/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "transaction_iterator.hpp"
#include "transaction.hpp"
#include "block.hpp"
#include "chain_access.hpp"
#include "input.hpp"
#include "output.hpp"

#define BLOCKSCI_WITHOUT_SINGLETON

namespace blocksci {
    
    TransactionIterator::TransactionIterator(const ChainAccess *access_, uint32_t txIndex_) : TransactionIterator(access_, txIndex_, access_->getBlockHeight(txIndex_)) {}
    
    TransactionIterator::TransactionIterator(const ChainAccess *access_, uint32_t txIndex, uint32_t blockNum_) : access(access_), currentTxPos(nullptr), currentTxIndex(txIndex), blockNum(blockNum_) {
        if (currentTxIndex < access->txCount()) {
            currentTxPos = access->getTxPos(currentTxIndex);
        } else {
            currentTxPos = nullptr;
        }
        updateNextBlock();
    }
    
    void TransactionIterator::increment() {
        auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        currentTxPos += sizeof(RawTransaction) +
        static_cast<size_t>(tx->inputCount) * sizeof(Input) +
        static_cast<size_t>(tx->outputCount) * sizeof(Output);
        currentTxIndex++;
        if (currentTxIndex == nextBlockFirst) {
            blockNum++;
            updateNextBlock();
        }
    }
    
    void TransactionIterator::decrement() {
        currentTxIndex--;
        if (currentTxIndex == prevBlockLast) {
            blockNum--;
            updateNextBlock();
        }
        currentTxPos = nullptr;
    }
    
    void TransactionIterator::advance(int amount) {
        currentTxIndex += static_cast<uint32_t>(amount);
        blockNum = access->getBlockHeight(currentTxIndex);
        updateNextBlock();
        currentTxPos = nullptr;
    }
    
    Transaction TransactionIterator::dereference() const {
        auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        return {rawTx, currentTxIndex, blockNum};
    }
    
    void TransactionIterator::updateNextBlock() {
        auto &block = access->getBlock(blockNum);
        prevBlockLast = block.firstTxIndex - 1;
        nextBlockFirst = blockNum < access->blockCount() - 1 ? block.firstTxIndex + static_cast<uint32_t>(block.size()) : std::numeric_limits<decltype(nextBlockFirst)>::max();
    }
}

