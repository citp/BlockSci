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
#include <blocksci/chain/chain_access.hpp>
#include <range/v3/view_facade.hpp>

namespace blocksci {
    class TransactionRange : public ranges::view_facade<TransactionRange> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        const char *currentTxPos;
        uint32_t currentTxIndex;
        uint32_t endTxIndex;
        BlockHeight blockNum;
        uint32_t nextBlockFirst;
        uint32_t prevBlockLast;
        
        bool equal(ranges::default_sentinel) const { return currentTxIndex == endTxIndex; }
        
        void next() {
            auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
            currentTxPos += sizeof(RawTransaction) +
            static_cast<size_t>(tx->inputCount) * sizeof(Inout) +
            static_cast<size_t>(tx->outputCount) * sizeof(Inout);
            currentTxIndex++;
            if (currentTxIndex == nextBlockFirst) {
                blockNum++;
                updateNextBlock();
            }
        }
        
        Transaction read() const {
            auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
            return {rawTx, currentTxIndex, blockNum, *access};
        }
        
        void prev();
        void advance(int amount);
        
        size_t distance_to(const TransactionRange &other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
        void updateNextBlock() {
            auto block = access->getBlock(blockNum);
            prevBlockLast = block->firstTxIndex - 1;
            nextBlockFirst = blockNum < access->blockCount() - 1 ? block->firstTxIndex + static_cast<uint32_t>(block->numTxes) : std::numeric_limits<decltype(nextBlockFirst)>::max();
        }
        
    public:
        TransactionRange() = default;
        TransactionRange(const ChainAccess &access, uint32_t begin, uint32_t end);
    };
    
    class RawTransactionRange : public ranges::view_facade<RawTransactionRange> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        mutable const char *currentTxPos;
        uint32_t currentTxIndex;
        uint32_t endTxIndex;
        
        const RawTransaction *read() const;
        bool equal(ranges::default_sentinel) const { return currentTxIndex == endTxIndex; }
        
        void next();
        void prev();
        void advance(int amount);
        
        size_t distance_to(const RawTransactionRange &other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
        void updateNextBlock();
        
    public:
        RawTransactionRange() = default;
        RawTransactionRange(const ChainAccess &access, uint32_t begin, uint32_t end);
    };
}



#endif /* transaction_range_hpp */
