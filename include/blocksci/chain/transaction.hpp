//
//  transaction.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_hpp
#define transaction_hpp

#include "output.hpp"
#include "input.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/raw_transaction.hpp>
#include <blocksci/index/mempool_index.hpp>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip_with.hpp>

namespace blocksci {
    class uint256;
    class HashIndex;
    
    struct BLOCKSCI_EXPORT InvalidHashException : public std::runtime_error {
        InvalidHashException() : std::runtime_error("No Match for hash") {}
    };
    
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
        
        Transaction(const RawTransaction *data_, uint32_t txNum_, BlockHeight blockHeight_, DataAccess &access_) : access(&access_), data(data_), sequenceNumbers(access_.getChain().getSequenceNumbers(txNum_)), txNum(txNum_), blockHeight(blockHeight_) {}
        
        Transaction(uint32_t index, DataAccess &access_) : Transaction(index, access_.getChain().getBlockHeight(index), access_) {}
        
        Transaction(uint32_t index, BlockHeight height, DataAccess &access_) : Transaction(access_.getChain().getTx(index), index, height, access_) {}
        
        Transaction(const uint256 &hash, DataAccess &access);
        Transaction(const std::string &hash, DataAccess &access) : Transaction(uint256S(hash), access) {}
        
        DataAccess &getAccess() const {
            return *access;
        }

        uint256 getHash() const {
            return *access->getChain().getTxHash(txNum);
        }
        
        ranges::optional<std::chrono::system_clock::time_point> getTimeSeen() const {
            return access->getMempoolIndex().getTxTimestamp(txNum);
        }
        
        bool observedInMempool() const {
            return access->getMempoolIndex().observed(txNum);
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
        
        int64_t fee() const {
            if (isCoinbase()) {
                return 0;
            } else {
                int64_t total = 0;
                for (auto &input : rawInputs()) {
                    total += input.getValue();
                }
                for (auto &vpubold : vpubolds()) {
                    total -= vpubold;
                }
                for (auto &output : rawOutputs()) {
                    total -= output.getValue();
                }
                for (auto &vpubnew : vpubnews()) {
                    total += vpubnew;
                }
                return total;
            }
        }
        
        int64_t totalVpubold() const {
			int64_t total = 0;
			for (auto &vpubold : vpubolds()) {
				total += vpubold;
			}
			return total;
		}
		
		int64_t totalVpubnew() const {
			int64_t total = 0;
			for (auto &vpubnew : vpubnews()) {
				total += vpubnew;
			}
			return total;
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
        
        uint16_t vpubCount() const {
            return data->vpubCount;
        }
        
        int64_t valueBalance() const {
			return data->valueBalance;
		}
		
		uint16_t shieldedSpendCount() const {
            return data->nShieldedSpend;
        }
        
        uint16_t shieldedOutputCount() const {
            return data->nShieldedOutput;
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
        
        ranges::iterator_range<const uint64_t *> vpubolds() const {
			auto pos = reinterpret_cast<const char *>(data) + sizeof(RawTransaction) + outputCount() * sizeof(Inout) + inputCount() * sizeof(Inout);
			return ranges::make_iterator_range(reinterpret_cast<const uint64_t *>(pos), reinterpret_cast<const uint64_t *>(pos + vpubCount() * sizeof(uint64_t)));
		}
		
		ranges::iterator_range<const uint64_t *> vpubnews() const {
			auto pos = reinterpret_cast<const char *>(data) + sizeof(RawTransaction) + outputCount() * sizeof(Inout) + inputCount() * sizeof(Inout) + vpubCount() * sizeof(uint64_t);
			return ranges::make_iterator_range(reinterpret_cast<const uint64_t *>(pos), reinterpret_cast<const uint64_t *>(pos + vpubCount() * sizeof(uint64_t)));
		}
		
		bool isShielded() const {
				return vpubCount() != 0 or shieldedSpendCount() != 0 or shieldedOutputCount() != 0;
		}
		
		bool isSproutShielded() const {
				return vpubCount() != 0;
		}
		
		bool isSaplingShielded() const {
				return shieldedSpendCount() != 0 or shieldedOutputCount() != 0;
		}
        
        bool isCoinbase() const {
            return inputCount() == 0 && !isShielded();
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
    
    inline bool BLOCKSCI_EXPORT includesOutputOfType(const Transaction &tx, AddressType::Enum type) {
        return ranges::any_of(tx.outputs(), [=](const Output &output) {
            return output.getType() == type;
        });
    }
    
    ranges::optional<Output> BLOCKSCI_EXPORT getOpReturn(const Transaction &tx);

    inline std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const Transaction &tx) {
        return os << tx.toString();
    }
    
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