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
    
    RawTransactionRange::RawTransactionRange(const ChainAccess &access_, uint32_t begin, uint32_t end) : access(&access_), currentTxPos(reinterpret_cast<const char *>(access->getTx(begin))), currentTxIndex(begin), endTxIndex(end) {
    }
    
    void RawTransactionRange::next() {
        auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        currentTxPos += sizeof(RawTransaction) +
        static_cast<size_t>(tx->inputCount) * sizeof(Inout) +
        static_cast<size_t>(tx->outputCount) * sizeof(Inout);
        currentTxIndex++;
    }
    
    void RawTransactionRange::prev() {
        currentTxIndex--;
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    void RawTransactionRange::advance(int amount) {
        currentTxIndex = static_cast<uint32_t>(static_cast<int>(currentTxIndex) + amount);
        currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
    }
    
    const RawTransaction *RawTransactionRange::read() const {
        return reinterpret_cast<const RawTransaction *>(currentTxPos);
    }
}
