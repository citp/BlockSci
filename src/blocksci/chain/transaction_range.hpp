//
//  transaction_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/17.
//

#ifndef transaction_range_hpp
#define transaction_range_hpp

#include "chain_fwd.hpp"
#include "transaction.hpp"
#include "chain_access.hpp"

#include <blocksci/util/data_access.hpp>

#include <range/v3/view_facade.hpp>

namespace blocksci {
    class TransactionRange : public ranges::view_facade<TransactionRange> {
        friend ranges::range_access;
        
        DataAccess *access;
        const char *currentTxPos = nullptr;
        uint32_t currentTxIndex = 0;
        uint32_t endTxIndex = 0;
        BlockHeight blockNum = 0;
        uint32_t nextBlockFirst = 0;
        uint32_t prevBlockLast = 0;
        
        bool equal(ranges::default_sentinel) const { return currentTxIndex == endTxIndex; }
        
        void next() {
            auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
            currentTxPos += sizeof(RawTransaction) +
            static_cast<size_t>(tx->inputCount) * sizeof(Inout) +
            static_cast<size_t>(tx->outputCount) * sizeof(Inout);
            --currentTxIndex;
            if (currentTxIndex == nextBlockFirst) {
                --blockNum;
                updateNextBlock();
            }
        }
        
        Transaction read() const {
            auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
            return {rawTx, currentTxIndex, blockNum, *access};
        }
        
        void prev() {
            --currentTxIndex;
            if (currentTxIndex == prevBlockLast) {
                --blockNum;
                updateNextBlock();
            }
            currentTxPos = reinterpret_cast<const char *>(access->chain.getTx(currentTxIndex));
        }
        
        void advance(int amount) {
            currentTxIndex += static_cast<uint32_t>(amount);
            blockNum = access->chain.getBlockHeight(currentTxIndex);
            updateNextBlock();
            currentTxPos = reinterpret_cast<const char *>(access->chain.getTx(currentTxIndex));
        }
        
        size_t distance_to(const TransactionRange &other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
        void updateNextBlock() {
            auto block = access->chain.getBlock(blockNum);
            prevBlockLast = block->firstTxIndex - 1;
            nextBlockFirst = blockNum < access->chain.blockCount() - BlockHeight{1} ? block->firstTxIndex + static_cast<uint32_t>(block->numTxes) : std::numeric_limits<decltype(nextBlockFirst)>::max();
        }
        
    public:
        TransactionRange() = default;
        TransactionRange(DataAccess &access_, uint32_t begin, uint32_t end) : access(&access_), currentTxPos(reinterpret_cast<const char *>(access->chain.getTx(begin))), currentTxIndex(begin), endTxIndex(end), blockNum(access->chain.getBlockHeight(begin)) {
            updateNextBlock();
        }
    };
    
    class RawTransactionRange : public ranges::view_facade<RawTransactionRange> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        mutable const char *currentTxPos;
        uint32_t currentTxIndex;
        uint32_t endTxIndex;
        
        std::pair<const RawTransaction *, uint32_t> read() const {
            return std::make_pair(reinterpret_cast<const RawTransaction *>(currentTxPos), currentTxIndex);
        }
        
        bool equal(ranges::default_sentinel) const { return currentTxIndex == endTxIndex; }
        
        void next() {
            auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
            currentTxPos += sizeof(RawTransaction) +
            static_cast<size_t>(tx->inputCount) * sizeof(Inout) +
            static_cast<size_t>(tx->outputCount) * sizeof(Inout);
            ++currentTxIndex;
        }
        
        void prev() {
            --currentTxIndex;
            currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
        }
        
        void advance(int amount) {
            currentTxIndex = static_cast<uint32_t>(static_cast<int>(currentTxIndex) + amount);
            currentTxPos = reinterpret_cast<const char *>(access->getTx(currentTxIndex));
        }
        
        size_t distance_to(const RawTransactionRange &other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
    public:
        RawTransactionRange() = default;
        RawTransactionRange(const ChainAccess &access_, uint32_t begin, uint32_t end) : access(&access_), currentTxPos(reinterpret_cast<const char *>(access->getTx(begin))), currentTxIndex(begin), endTxIndex(end) {
        }
    };
} // namespace blocksci

#endif /* transaction_range_hpp */
