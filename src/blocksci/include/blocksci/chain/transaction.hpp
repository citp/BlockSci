//
//  transaction.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_hpp
#define transaction_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/raw_transaction.hpp>

#include "output.hpp"
#include "input.hpp"

#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip_with.hpp>

namespace blocksci {
    class uint256;
    class HashIndex;
    
    struct BLOCKSCI_EXPORT InvalidHashException : public std::runtime_error {
        InvalidHashException() : std::runtime_error("No Match for hash") {}
    };
    
    namespace internal {
        inline uint32_t getTxIndex(const uint256 &hash, HashIndex &index) {
            auto txIndex = index.getTxIndex(hash);
            if (txIndex == 0) {
                throw InvalidHashException();
            }
            return txIndex;
        }
    }
    
    class BLOCKSCI_EXPORT Transaction {
    private:
        DataAccess *access;
        const RawTransaction *data;
        const uint32_t *sequenceNumbers;
        friend TransactionSummary;
    public:
        uint32_t txNum;
        BlockHeight blockHeight;
        
        Transaction() = default;
        
        Transaction(const RawTransaction *data_, uint32_t txNum_, BlockHeight blockHeight_, DataAccess &access_) : access(&access_), data(data_), sequenceNumbers(access_.chain.getSequenceNumbers(txNum_)), txNum(txNum_), blockHeight(blockHeight_) {}
        
        Transaction(uint32_t index, DataAccess &access_) : Transaction(index, access_.chain.getBlockHeight(index), access_) {}
        
        Transaction(uint32_t index, BlockHeight height, DataAccess &access_) : Transaction(access_.chain.getTx(index), index, height, access_) {}
        
        Transaction(const uint256 &hash, DataAccess &access) : Transaction(internal::getTxIndex(hash, access.hashIndex), access) {}
        Transaction(const std::string &hash, DataAccess &access) : Transaction(uint256S(hash), access) {}
        
        DataAccess &getAccess() const {
            return *access;
        }

        uint256 getHash() const {
            return *access->chain.getTxHash(txNum);
        }
        
        ranges::optional<std::chrono::system_clock::time_point> getTimeSeen() const {
            return access->mempoolIndex.getTxTimestamp(txNum);
        }
        
        bool observedInMempool() const {
            return access->mempoolIndex.observed(txNum);
        }
        
        std::string toString() const {
            std::stringstream ss;
            ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << blockHeight <<", tx_index=" << txNum << ")";
            return ss.str();
        }
        
        std::vector<OutputPointer> getOutputPointers(const InputPointer &pointer) const;
        std::vector<InputPointer> getInputPointers(const OutputPointer &pointer) const;
        
        uint32_t baseSize() const {
            return data->baseSize;
        }
        
        uint32_t totalSize() const {
            return data->realSize;
        }
        
        uint32_t virtualSize() const {
            return (weight() + 3) / 4;
        }
        
        uint32_t weight() const {
            return data->realSize + 3 * data->baseSize;
        }
        
        uint32_t sizeBytes() const {
            return virtualSize();
        }
        
        uint32_t locktime() const {
            return data->locktime;
        }
        
        uint16_t inputCount() const {
            return data->inputCount;
        }
        
        uint16_t outputCount() const {
            return data->outputCount;
        }
        
        ranges::iterator_range<const Inout *> rawOutputs() const {
            return ranges::make_iterator_range(data->beginOutputs(), data->endOutputs());
        }
        
        ranges::iterator_range<const Inout *> rawInputs() const {
            return ranges::make_iterator_range(data->beginInputs(), data->endInputs());
        }

        auto outputs() const {
            auto dataAccess = access;
            uint32_t txIndex = txNum;
            BlockHeight height = blockHeight;
            return ranges::view::zip_with([dataAccess, txIndex, height](uint16_t outputNum, const Inout &inout) {
                return Output({txIndex, outputNum}, height, inout, *dataAccess);
            }, ranges::view::ints(uint16_t{0}, outputCount()), rawOutputs());
        }
        
        auto inputs() const {
            auto dataAccess = access;
            uint32_t txIndex = txNum;
            BlockHeight height = blockHeight;
            auto seq = sequenceNumbers;
            return ranges::view::zip_with([dataAccess, txIndex, height, seq](uint16_t inputNum, const Inout &inout) {
                return Input({txIndex, inputNum}, height, inout, &seq[inputNum], *dataAccess);
            }, ranges::view::ints(uint16_t{0}, inputCount()), rawInputs());
        }
        
        bool isCoinbase() const {
            return inputCount() == 0;
        }
        
        Block block() const;
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const Transaction& a, const Transaction& b) {
        return a.txNum == b.txNum;
    }

    inline bool BLOCKSCI_EXPORT operator!=(const Transaction& a, const Transaction& b) {
        return a.txNum != b.txNum;
    }
    
    inline bool BLOCKSCI_EXPORT operator<(const Transaction& a, const Transaction& b) {
        return a.txNum < b.txNum;
    }

    inline bool BLOCKSCI_EXPORT operator<=(const Transaction& a, const Transaction& b) {
        return a.txNum <= b.txNum;
    }

    inline bool BLOCKSCI_EXPORT operator>(const Transaction& a, const Transaction& b) {
        return a.txNum > b.txNum;
    }

    inline bool BLOCKSCI_EXPORT operator>=(const Transaction& a, const Transaction& b) {
        return a.txNum >= b.txNum;
    }
    
    using input_range = decltype(std::declval<Transaction>().inputs());
    using output_range = decltype(std::declval<Transaction>().outputs());
    
    bool BLOCKSCI_EXPORT hasFeeGreaterThan(Transaction &tx, int64_t txFee);
    
    ranges::optional<Output> BLOCKSCI_EXPORT getOpReturn(const Transaction &tx);

    inline std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const Transaction &tx) {
        return os << tx.toString();
    }
} // namespace blocksci


namespace std {
    template <>
    struct BLOCKSCI_EXPORT hash<blocksci::Transaction> {
        typedef blocksci::Transaction argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return static_cast<size_t>(b.txNum);
        }
    };
} // namespace std


#endif /* transaction_hpp */
