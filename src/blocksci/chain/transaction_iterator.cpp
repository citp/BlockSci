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
        updateNextBlock();
    }
    
    void TransactionIterator::increment() {
        auto &txPos = getTxPos();
        auto tx = reinterpret_cast<const RawTransaction *>(txPos);
        txPos += sizeof(RawTransaction) +
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
        updateTxPos();
    }
    
    void TransactionIterator::advance(int amount) {
        currentTxIndex += static_cast<uint32_t>(amount);
        blockNum = access->getBlockHeight(currentTxIndex);
        updateNextBlock();
        updateTxPos();
    }
    
    Transaction TransactionIterator::dereference() const {
        return {reinterpret_cast<const RawTransaction *>(getTxPos()), currentTxIndex, blockNum};
    }
    
    void TransactionIterator::updateNextBlock() {
        if (blockNum < access->getBlocks().size()) {
            auto &block = access->getBlock(blockNum);
            prevBlockLast = block.firstTxIndex - 1;
            nextBlockFirst = block.firstTxIndex + static_cast<uint32_t>(block.size());
        } else {
            auto &prevBlock = access->getBlock(blockNum - 1);
            prevBlockLast = prevBlock.firstTxIndex + static_cast<uint32_t>(prevBlock.size()) - 1;
            nextBlockFirst = std::numeric_limits<decltype(nextBlockFirst)>::max();
        }
    }
    
    void TransactionIterator::updateTxPos() {
        currentTxPos = access->getTxPos(currentTxIndex);
    }
    
    const char *&TransactionIterator::getTxPos() const {
        if (currentTxPos == nullptr) {
            currentTxPos = access->getTxPos(currentTxIndex);
        }
        return currentTxPos;
    }
}

