//
//  block.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_hpp
#define block_hpp

#include "transaction.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/raw_block.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>

#include <unordered_map>
#include <chrono>

namespace blocksci {
    class DataAccess;
    
    class BLOCKSCI_EXPORT Block : public ranges::view_facade<Block> {
        friend ranges::range_access;
        
        DataAccess *access;
        const RawBlock *rawBlock;
        BlockHeight blockNum;
        
        struct cursor {
        private:
            const Block *block;
            const char *currentTxPos;
            uint32_t currentTxIndex;
        public:
            cursor() = default;
            cursor(const Block &block_, uint32_t txNum) : block(&block_), currentTxIndex(txNum) {
                if (currentTxIndex < block->access->getChain().txCount()) {
                    currentTxPos = reinterpret_cast<const char *>(block->access->getChain().getTx(currentTxIndex));
                } else {
                    currentTxPos = nullptr;
                }
            }
            
            Transaction read() const {
                auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
                return {rawTx, currentTxIndex, block->height(), *block->access};
            }
            
            bool equal(cursor const &that) const {
                return currentTxIndex == that.currentTxIndex;
            }
            
            bool equal(ranges::default_sentinel) const {
                return currentTxIndex == block->endTxIndex();
            }
            
            void next() {
                auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
                currentTxPos += sizeof(RawTransaction) +
                static_cast<size_t>(tx->inputCount) * sizeof(Inout) +
                static_cast<size_t>(tx->outputCount) * sizeof(Inout) +
                static_cast<size_t>(tx->vpubCount) * sizeof(uint64_t) * 2;
                currentTxIndex++;
            }
            
            int distance_to(cursor const &that) const {
                return static_cast<int>(that.currentTxIndex) - static_cast<int>(currentTxIndex);
            }
            
            int distance_to(ranges::default_sentinel) const {
                return static_cast<int>(block->endTxIndex()) - static_cast<int>(currentTxIndex);
            }
            
            void prev() {
                currentTxIndex--;
                currentTxPos = reinterpret_cast<const char *>(block->access->getChain().getTx(currentTxIndex));
            }
            
            void advance(int amount) {
                currentTxIndex += static_cast<uint32_t>(amount);
                if (currentTxIndex < block->access->getChain().txCount()) {
                    currentTxPos = reinterpret_cast<const char *>(block->access->getChain().getTx(currentTxIndex));
                } else {
                    currentTxPos = nullptr;
                }
            }
        };
        
        cursor begin_cursor() const {
            return cursor(*this, firstTxIndex());
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
        }
        
    public:
        Block() = default;
        Block(BlockHeight blockNum_, DataAccess &access_) : access(&access_), rawBlock(access->getChain().getBlock(blockNum_)), blockNum(blockNum_) {
        }
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        bool operator==(const Block &other) const {
            return *rawBlock == *other.rawBlock;
        }

        bool operator!=(const Block &other) const {
            return !(*rawBlock == *other.rawBlock);
        }
        
        uint256 getHash() const {
            return rawBlock->hash;
        }

        auto allInputs() const {
            return *this | ranges::view::transform([](const Transaction &tx) { return tx.inputs(); }) | ranges::view::join;
        }
        
        auto allOutputs() const {
            return *this | ranges::view::transform([](const Transaction &tx) { return tx.outputs(); }) | ranges::view::join;
        }
        
        uint32_t firstTxIndex() const {
            return rawBlock->firstTxIndex;
        }
        
        uint32_t endTxIndex() const {
            return rawBlock->firstTxIndex + rawBlock->numTxes;
        }
        
        BlockHeight height() const {
            return blockNum;
        }
        
        int32_t version() const {
            return rawBlock->version;
        }
        
        uint32_t timestamp() const {
            return rawBlock->timestamp;
        }
        
        ranges::optional<std::chrono::system_clock::time_point> getTimeSeen() const {
            return access->getMempoolIndex().getBlockTimestamp(blockNum);
        }
        
        uint32_t bits() const {
            return rawBlock->bits;
        }
        
        uint256 nonce() const {
            return rawBlock->nonce;
        }
        
        double difficulty() const {
			uint32_t nbits = bits();

			uint32_t powLimit = 520617983;
			
			int nShift = (nbits >> 24) & 0xff;
			int nShiftAmount = (powLimit >> 24) & 0xff;

			double dDiff =
				(double)(powLimit & 0x00ffffff) / 
				(double)(nbits & 0x00ffffff);

			while (nShift < nShiftAmount)
			{
				dDiff *= 256.0;
				nShift++;
			}
			while (nShift > nShiftAmount)
			{
				dDiff /= 256.0;
				nShift--;
			}

			return dDiff;
		}
        
        uint32_t baseSize() const {
            return rawBlock->baseSize;
        }
        
        uint32_t totalSize() const {
            return rawBlock->realSize;
        }
        
        uint32_t virtualSize() const {
            return (weight() + 3) / 4;
        }
        
        uint32_t weight() const {
            return baseSize() * 3 + totalSize();
        }
        
        uint32_t sizeBytes() const {
            return virtualSize();
        }

        Block nextBlock() const {
            return {blockNum + BlockHeight(1), *access};
        }
        
        Block prevBlock() const {
            return {blockNum - BlockHeight(1), *access};
        }
        
        const std::string getHeaderHash() const {
            return rawBlock->hash.GetHex();
        }
        
        std::chrono::system_clock::time_point getTime() const {
            return std::chrono::system_clock::from_time_t(static_cast<time_t>(rawBlock->timestamp));
        }
        
        std::string toString() const {
            std::stringstream ss;
            ss << "Block(numTxes=" << rawBlock->numTxes <<", height=" << blockNum <<", header_hash=" << rawBlock->hash.GetHex() << ", version=" << rawBlock->version <<", timestamp=" << rawBlock->timestamp << ", bits=" << rawBlock->bits << ", nonce=" << rawBlock->nonce.GetHex() << ")";
            return ss.str();
        }
        
        const std::string coinbaseParam() const {
            auto coinbase = getCoinbase();
            return std::string(coinbase.begin(), coinbase.end());
        }
        
        std::vector<unsigned char> getCoinbase() const {
            return access->getChain().getCoinbase(rawBlock->coinbaseOffset);
        }
        
        Transaction coinbaseTx() const {
            return (*this)[0];
        }
    };
    
    inline bool BLOCKSCI_EXPORT isSegwit(const Block &block) {
        return isSegwitMarker(block.coinbaseTx());
    }
    
    inline std::unordered_map<AddressType::Enum, int64_t> BLOCKSCI_EXPORT netAddressTypeValue(const Block &block) {
        std::unordered_map<AddressType::Enum, int64_t> net;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                net[output.getType()] += output.getValue();
            }
            RANGES_FOR(auto input, tx.inputs()) {
                net[input.getType()] -= input.getValue();
            }
        }
        return net;
    }
    
    inline std::unordered_map<std::string, int64_t> BLOCKSCI_EXPORT netFullTypeValue(const Block &block) {
        std::unordered_map<std::string, int64_t> net;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                net[output.getAddress().fullType()] += output.getValue();
            }
            RANGES_FOR(auto input, tx.inputs()) {
                net[input.getAddress().fullType()] -= input.getValue();
            }
        }
        return net;
    }
} // namespace blocksci

inline std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const blocksci::Block &block) {
    os << block.toString();
    return os;
}

namespace std {
    template <>
    struct hash<blocksci::Block> {
        typedef blocksci::Block argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return static_cast<size_t>(b.height());
        }
    };
} // namespace std

#endif /* block_hpp */