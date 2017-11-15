//
//  transaction_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/17.
//

#ifndef transaction_range_hpp
#define transaction_range_hpp

#include "transaction.hpp"
#include <range/v3/view_facade.hpp>

namespace blocksci {
    class TransactionRange : public ranges::view_facade<TransactionRange> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        mutable const char *currentTxPos;
        uint32_t currentTxIndex;
        uint32_t endTxIndex;
        uint32_t blockNum;
        uint32_t nextBlockFirst;
        uint32_t prevBlockLast;
        
        Transaction read() const;
        bool equal(ranges::default_sentinel) const { return currentTxIndex == endTxIndex; }
        
        void next();
        void prev();
        void advance(int amount);
        
        size_t distance_to(const TransactionRange &other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
        void updateNextBlock();
        
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
