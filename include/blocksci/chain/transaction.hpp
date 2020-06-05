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

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>
#include <blocksci/core/raw_transaction.hpp>
#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/transaction_data.hpp>

#include <range/v3/utility/optional.hpp>

#include <chrono>

namespace blocksci {
    class uint256;
    class DataAccess;
    
    struct BLOCKSCI_EXPORT InvalidHashException : public std::runtime_error {
        InvalidHashException() : std::runtime_error("No Match for hash") {}
    };

    /** Represents a Transaction of the blockchain
     *
     * Transaction data is stored as RawTransaction in chain/tx_data.dat. Inputs and outputs are stored as
     * Inout objects in the same file, see ChainAccess for details.
     */
    class BLOCKSCI_EXPORT Transaction {
    private:
        DataAccess *access;
        TxData data;
        uint32_t maxTxCount;
        mutable BlockHeight blockHeight;
        friend TransactionRange;
    public:
        /** Blockchain-wide transaction number in the same order they appear in the blockchain, also called transaction index */
        uint32_t txNum;
        
        
        Transaction() = default;
        
        Transaction(const TxData &data_, uint32_t txNum_, BlockHeight blockHeight_, uint32_t maxTxCount_, DataAccess &access_) : access(&access_), data(data_), maxTxCount(maxTxCount_), blockHeight(blockHeight_), txNum(txNum_) {}
        
        Transaction(uint32_t index, BlockHeight height, DataAccess &access_);
        
        Transaction(uint32_t index, DataAccess &access_);
        
        Transaction(const uint256 &hash, DataAccess &access);
        Transaction(const std::string &hash, DataAccess &access);
        
        DataAccess &getAccess() const {
            return *access;
        }

        uint256 getHash() const {
            return *data.hash;
        }
        
        int32_t getVersion() const {
            return *data.version;
        }
        
        BlockHeight calculateBlockHeight() const;
        
        BlockHeight getBlockHeight() const {
            if (blockHeight == -1) {
                blockHeight = calculateBlockHeight();
            }
            return blockHeight;
        }
        
        // Mempool data access
        ranges::optional<std::chrono::system_clock::time_point> getTimeSeen() const;
        ranges::optional<uint32_t> getTimestampSeen() const;
        bool observedInMempool() const;
        
        std::string toString() const;
        
        std::vector<OutputPointer> getOutputPointers(const InputPointer &pointer) const;
        std::vector<InputPointer> getInputPointers(const OutputPointer &pointer) const;
        
        uint32_t baseSize() const {
            return data.rawTx->baseSize;
        }
        
        uint32_t totalSize() const {
            return data.rawTx->realSize;
        }
        
        uint32_t virtualSize() const {
            return (weight() + 3) / 4;
        }
        
        uint32_t weight() const {
            return data.rawTx->realSize + 3 * data.rawTx->baseSize;
        }
        
        uint32_t sizeBytes() const {
            return virtualSize();
        }
        
        int64_t fee() const {
            if (isCoinbase()) {
                return 0;
            } else {
                int64_t total = 0;
                for (auto input : inputs()) {
                    total += input.getValue();
                }
                for (auto output : outputs()) {
                    total -= output.getValue();
                }
                return total;
            }
        }
        
        uint32_t locktime() const {
            return data.rawTx->locktime;
        }
        
        uint16_t inputCount() const {
            return data.rawTx->inputCount;
        }
        
        uint16_t outputCount() const {
            return data.rawTx->outputCount;
        }
        
        OutputRange outputs() const {
            return {data.rawTx->beginOutputs(), getBlockHeight(), txNum, outputCount(), maxTxCount, access};
        }
        
        InputRange inputs() const {
            return {data.rawTx->beginInputs(), data.spentOutputNums, data.sequenceNumbers, getBlockHeight(), txNum, inputCount(), maxTxCount, access};
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
    
    bool BLOCKSCI_EXPORT includesOutputOfType(const Transaction &tx, AddressType::Enum type);
    
    ranges::optional<Output> BLOCKSCI_EXPORT getOpReturn(const Transaction &tx);

    std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const Transaction &tx);
    
    bool BLOCKSCI_EXPORT isSegwitMarker(const Transaction &tx);
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
