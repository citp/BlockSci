//
//  transaction_iterator.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/20/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_iterator_hpp
#define transaction_iterator_hpp

#include "transaction.hpp"
#include <boost/iterator/iterator_facade.hpp>

namespace blocksci {
    
    class TransactionIterator : public boost::iterator_facade<TransactionIterator, Transaction, boost::random_access_traversal_tag, Transaction> {
        const ChainAccess *access;
        mutable const char *currentTxPos;
        uint32_t currentTxIndex;
        uint32_t blockNum;
        uint32_t nextBlockFirst;
        uint32_t prevBlockLast;
        
    public:
        TransactionIterator() {}
        TransactionIterator(const ChainAccess *access, uint32_t txIndex);
        TransactionIterator(const ChainAccess *access, uint32_t txIndex, uint32_t blockNum);
        
        uint32_t getTxIndex() {
            return currentTxIndex;
        }
        
    private:
        friend class boost::iterator_core_access;
        
        void increment();
        
        void decrement();
        
        void advance(int amount);
        
        bool equal(TransactionIterator const& other) const {
            return currentTxIndex == other.currentTxIndex;
        }
        
        size_t distance_to(TransactionIterator const& other) const {
            return static_cast<size_t>(other.currentTxIndex) - static_cast<size_t>(currentTxIndex);
        }
        
        Transaction dereference() const;
        
        void updateNextBlock();
    };
}

#endif /* transaction_iterator_hpp */
