//
//  transaction_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/17.
//

#include "transaction_range.hpp"
#include "block.hpp"
#include "chain_access.hpp"
#include "input.hpp"
#include "output.hpp"

namespace blocksci {
    
    TransactionRange::TransactionRange(const ChainAccess &access_, uint32_t begin, uint32_t end) : access(&access_), currentTxPos(reinterpret_cast<const char *>(access->getTx(begin))), currentTxIndex(begin), endTxIndex(end), blockNum(access->getBlockHeight(begin)) {
        updateNextBlock();
    }
    
    void TransactionRange::next() {
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
    
    void TransactionRange::prev() {
        currentTxIndex--;
        if (currentTxIndex == prevBlockLast) {
            blockNum--;
            updateNextBlock();
        }
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    void TransactionRange::advance(int amount) {
        currentTxIndex += static_cast<uint32_t>(amount);
        blockNum = access->getBlockHeight(currentTxIndex);
        updateNextBlock();
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    Transaction TransactionRange::read() const {
        auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        return {rawTx, currentTxIndex, blockNum, *access};
    }
    
    void TransactionRange::updateNextBlock() {
        auto block = access->getBlock(blockNum);
        prevBlockLast = block->firstTxIndex - 1;
        nextBlockFirst = blockNum < access->blockCount() - 1 ? block->firstTxIndex + static_cast<uint32_t>(block->numTxes) : std::numeric_limits<decltype(nextBlockFirst)>::max();
    }
    
    RawTransactionRange::RawTransactionRange(const ChainAccess &access_, uint32_t begin, uint32_t end) : access(&access_), currentTxPos(reinterpret_cast<const char *>(access->getTx(begin))), currentTxIndex(begin), endTxIndex(end) {
    }
    
    void RawTransactionRange::next() {
        auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        currentTxPos += sizeof(RawTransaction) +
        static_cast<size_t>(tx->inputCount) * sizeof(Input) +
        static_cast<size_t>(tx->outputCount) * sizeof(Output);
        currentTxIndex++;
    }
    
    void RawTransactionRange::prev() {
        currentTxIndex--;
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    void RawTransactionRange::advance(int amount) {
        currentTxIndex += amount;
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    const RawTransaction *RawTransactionRange::read() const {
        return reinterpret_cast<const RawTransaction *>(currentTxPos);
    }
}
