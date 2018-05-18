//
//  transaction_range.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/transaction_range.hpp>
#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>

#include <range/v3/range_concepts.hpp>

namespace blocksci {
    
    void TransactionRange::iterator::resetTx() {
        tx.data.rawTx = tx.access->getChain().getTx(tx.txNum);
    }
    
    void TransactionRange::iterator::resetTxData() {
        tx.data = tx.access->getChain().getTxData(tx.txNum);
    }
    
    void TransactionRange::iterator::resetHeight() {
        tx.blockHeight = tx.access->getChain().getBlockHeight(tx.txNum);
    }
    
    void TransactionRange::iterator::updateNextBlock() {
        auto block = tx.access->getChain().getBlock(tx.blockHeight);
        prevBlockLast = block->firstTxIndex - 1;
        nextBlockFirst = tx.blockHeight < tx.access->getChain().blockCount() - BlockHeight{1} ? block->firstTxIndex + static_cast<uint32_t>(block->txCount) : std::numeric_limits<decltype(nextBlockFirst)>::max();
    }
    
    TransactionRange::iterator::value_type TransactionRange::iterator::operator[](difference_type i) const {
        auto index = tx.txNum + i;
        auto data = tx.getAccess().getChain().getTxData(index);
        auto height = tx.getAccess().getChain().getBlockHeight(index);
        return {data, index, height, tx.maxTxCount, tx.getAccess()};
    }

    Transaction TransactionRange::operator[](uint32_t txIndex) const {
        auto index = slice.start + txIndex;
        auto data = firstTx.getAccess().getChain().getTxData(index);
        auto height = firstTx.getAccess().getChain().getBlockHeight(index);
        return {data, index, height, firstTx.maxTxCount, firstTx.getAccess()};
    }
    
    CONCEPT_ASSERT(ranges::BidirectionalRange<TransactionRange>());
    CONCEPT_ASSERT(ranges::BidirectionalIterator<TransactionRange::iterator>());
    CONCEPT_ASSERT(ranges::SizedSentinel<TransactionRange::iterator, TransactionRange::iterator>());
    CONCEPT_ASSERT(ranges::TotallyOrdered<TransactionRange::iterator>());
    CONCEPT_ASSERT(ranges::RandomAccessIterator<TransactionRange::iterator>());
    CONCEPT_ASSERT(ranges::RandomAccessRange<TransactionRange>());
    CONCEPT_ASSERT(ranges::SizedRange<TransactionRange>());
} // namespace blocksci
