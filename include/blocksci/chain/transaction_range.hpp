//
//  transaction_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/17.
//

#ifndef transaction_range_hpp
#define transaction_range_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/transaction.hpp>

namespace blocksci {
    /** Represents an iterable collection of contiguous Transaction objects */
    class BLOCKSCI_EXPORT TransactionRange {
    public:
        class iterator {
        public:
            using self_type = iterator;
            using value_type = Transaction;
            using pointer = Transaction;
            using reference = Transaction;
            using size_type = uint32_t;
            using difference_type = int;
            using iterator_category = std::random_access_iterator_tag;
            
            iterator() = default;
            
            iterator(const Transaction &tx_) : tx(tx_) {}
            
            self_type &operator+=(difference_type i) {
                if (i > 0) {
                    tx.txNum += static_cast<uint32_t>(i);
                } else {
                    tx.txNum -= static_cast<uint32_t>(-i);
                }
                resetTxData();
                if (tx.txNum >= nextBlockFirst) {
                    resetHeight();
                    updateNextBlock();
                }
                return *this;
            }
            
            self_type &operator-=(difference_type i) {
                if (i > 0) {
                    tx.txNum -= static_cast<uint32_t>(i);
                } else {
                    tx.txNum += static_cast<uint32_t>(-i);
                }
                resetTxData();
                if (tx.txNum <= prevBlockLast) {
                    resetHeight();
                    updateNextBlock();
                }
                return *this;
            }
            
            self_type &operator++() {
                ++tx.data;
                ++tx.txNum;
                if (tx.txNum == nextBlockFirst) {
                    ++tx.blockHeight;
                    updateNextBlock();
                }
                return *this;
            }
            
            self_type &operator--() {
                --tx.txNum;
                if (tx.txNum == prevBlockLast) {
                    --tx.blockHeight;
                    updateNextBlock();
                }
                resetTx();
                tx.data.sequenceNumbers -= tx.data.rawTx->inputCount;
                tx.data.spentOutputNums -= tx.data.rawTx->inputCount;
                --tx.data.version;
                --tx.data.hash;
                return *this;
            }
            
            self_type operator++(int) { self_type tmp = *this; this->operator++(); return tmp;}
            self_type operator--(int) { self_type tmp = *this; this->operator--(); return tmp; }
            self_type operator+(difference_type i) const { self_type tmp = *this; tmp += i; return tmp; }
            self_type operator-(difference_type i) const { self_type tmp = *this; tmp -= i; return tmp; }
            
            value_type operator*() const { return tx; }
            value_type operator[](size_type i) const;
            
            bool operator==(const self_type& rhs) const { return tx == rhs.tx; }
            bool operator!=(const self_type& rhs) const { return tx != rhs.tx; }
            bool operator<(const self_type& rhs) const { return tx < rhs.tx; }
            bool operator>(const self_type& rhs) const { return tx > rhs.tx; }
            bool operator<=(const self_type& rhs) const { return tx <= rhs.tx; }
            bool operator>=(const self_type& rhs) const { return tx >= rhs.tx; }
            difference_type operator-(const self_type& it) const {return static_cast<difference_type>(tx.txNum - it.tx.txNum);}
        private:
            Transaction tx;
            uint32_t nextBlockFirst = 0;
            uint32_t prevBlockLast = 0;
            
            // Reset just data.rawTx based on txIndex
            void resetTx();
            
            // Reset data based on txIndex
            void resetTxData();
            
            // Reset nextBlockFirst and prevBlockLast based on height
            void updateNextBlock();
            
            // Reset height based on txIndex
            void resetHeight();
        };
        
        using size_type = typename iterator::size_type;
        using difference_type = typename iterator::difference_type;
        
        struct Slice {
            size_type start;
            size_type stop;
        };
        
        TransactionRange() = default;
        TransactionRange(const Slice &slice_, const Transaction &firstTx_) : slice(slice_), firstTx(firstTx_) {}
        
        iterator begin() const {
            return {firstTx};
        }
        
        iterator end() const {
            return {{TxData{}, slice.stop, 0, firstTx.maxTxCount, firstTx.getAccess()}};
        }
        
        Transaction operator[](uint32_t txIndex) const;
        
        TransactionRange operator[](const Slice &sl) const {
            if (sl.start == sl.stop) {
                auto fakeTx = Transaction{TxData{}, slice.stop, 0, firstTx.maxTxCount, firstTx.getAccess()};
                return {{slice.start + sl.start, slice.start + sl.stop}, fakeTx};
            } else {
                return {{slice.start + sl.start, slice.start + sl.stop}, this->operator[](sl.start)};
            }
        }
        
        uint32_t size() const {
            return slice.stop - slice.start;
        }
        
        BlockHeight startHeight() const {
            return firstTx.blockHeight;
        }
        
        uint32_t firstTxIndex() const {
            return slice.start;
        }
        
        uint32_t endTxIndex() const {
            return slice.stop;
        }
        
        DataAccess &getAccess() const {
            return firstTx.getAccess();
        }
        
    private:
        Slice slice;
        Transaction firstTx;
    };
    
    inline TransactionRange::iterator::self_type BLOCKSCI_EXPORT operator+(TransactionRange::iterator::difference_type i, const TransactionRange::iterator &it) {
        return it + i;
    }
}

#endif /* transaction_range_hpp */
