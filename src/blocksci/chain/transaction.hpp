//
//  transaction.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_hpp
#define transaction_hpp

#include "raw_transaction.hpp"
#include "output.hpp"
#include "input.hpp"

#include <blocksci/util/data_access.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/address/address_info.hpp>

#include <range/v3/iterator_range.hpp>
#include <range/v3/view/zip_with.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/utility/optional.hpp>

#include <vector>

namespace blocksci {
    class uint256;
    class HashIndex;
    
    struct InvalidHashException : public std::runtime_error {
        InvalidHashException() : std::runtime_error("No Match for hash") {}
    };
    
    class Transaction {
    private:
        const DataAccess *access;
        const RawTransaction *data;
        const uint32_t *sequenceNumbers;
        friend TransactionSummary;
    public:
        uint32_t txNum;
        BlockHeight blockHeight;
        
        Transaction() = default;
        
        Transaction(const RawTransaction *data_, uint32_t txNum_, BlockHeight blockHeight_, const DataAccess &access_) : access(&access_), data(data_), sequenceNumbers(access_.chain->getSequenceNumbers(txNum_)), txNum(txNum_), blockHeight(blockHeight_) {}
        
        Transaction(uint32_t index, const DataAccess &access_) : Transaction(index, access_.chain->getBlockHeight(index), access_) {}
        
        Transaction(uint32_t index, BlockHeight height, const DataAccess &access_) : Transaction(access_.chain->getTx(index), index, height, access_) {}
        
        Transaction(uint256 hash, const DataAccess &access);
        Transaction(std::string hash, const DataAccess &access);
        
        const DataAccess &getAccess() const {
            return *access;
        }

        
        uint256 getHash() const;
        std::string toString() const;
        
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
            auto &firstOut = data->getOutput(0);
            return ranges::make_iterator_range(&firstOut, &firstOut + outputCount());
        }
        
        ranges::iterator_range<const Inout *> rawInputs() const {
            auto &firstIn = data->getInput(0);
            return ranges::make_iterator_range(&firstIn, &firstIn + inputCount());
        }
        
        auto outputs() const {
            auto dataAccess = access;
            uint32_t txIndex = txNum;
            BlockHeight height = blockHeight;
            return ranges::view::zip_with([dataAccess, txIndex, height](uint16_t outputNum, const Inout &inout) {
                return Output({txIndex, outputNum}, height, inout, *dataAccess);
            }, ranges::view::iota(uint16_t{0}, outputCount()), rawOutputs());
        }
        
        auto inputs() const {
            auto dataAccess = access;
            uint32_t txIndex = txNum;
            BlockHeight height = blockHeight;
            auto seq = sequenceNumbers;
            return ranges::view::zip_with([dataAccess, txIndex, height, seq](uint16_t inputNum, const Inout &inout) {
                return Input({txIndex, inputNum}, height, inout, &seq[inputNum], *dataAccess);
            }, ranges::view::iota(uint16_t{0}, inputCount()), rawInputs());
        }
        
        bool isCoinbase() const {
            return inputCount() == 0;
        }
        
        Block block() const;
    };
    
    inline bool operator==(const Transaction& a, const Transaction& b) {
        return a.txNum == b.txNum;
    }

    inline bool operator!=(const Transaction& a, const Transaction& b) {
        return a.txNum != b.txNum;
    }
    
    inline bool operator<(const Transaction& a, const Transaction& b) {
        return a.txNum < b.txNum;
    }

    inline bool operator<=(const Transaction& a, const Transaction& b) {
        return a.txNum <= b.txNum;
    }

    inline bool operator>(const Transaction& a, const Transaction& b) {
        return a.txNum > b.txNum;
    }

    inline bool operator>=(const Transaction& a, const Transaction& b) {
        return a.txNum >= b.txNum;
    }
    
    using input_range = decltype(std::declval<Transaction>().inputs());
    using output_range = decltype(std::declval<Transaction>().outputs());
    
    bool hasFeeGreaterThan(Transaction &tx, uint64_t fee);
    
    ranges::optional<Output> getOpReturn(const Transaction &tx);

    inline std::ostream &operator<<(std::ostream &os, const Transaction &tx) { 
        return os << tx.toString();
    }
}


namespace std {
    template <>
    struct hash<blocksci::Transaction> {
        typedef blocksci::Transaction argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return static_cast<size_t>(b.txNum);
        }
    };
}


#endif /* transaction_hpp */
