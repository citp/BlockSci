//
//  block.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_hpp
#define block_hpp

#include "chain_fwd.hpp"
#include "transaction.hpp"
#include "raw_block.hpp"

#include <blocksci/util/data_access.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/address/address_types.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>

#include <unordered_map>
#include <chrono>

namespace blocksci {
    class DataAccess;
    
    class Block : public ranges::view_facade<Block> {
        friend ranges::range_access;
        
        const DataAccess *access;
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
                if (currentTxIndex < block->access->chain->txCount()) {
                    currentTxPos = reinterpret_cast<const char *>(block->access->chain->getTx(currentTxIndex));
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
                static_cast<size_t>(tx->outputCount) * sizeof(Inout);
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
                currentTxPos = reinterpret_cast<const char *>(block->access->chain->getTx(currentTxIndex));
            }
            
            void advance(int amount) {
                currentTxIndex += static_cast<uint32_t>(amount);
                if (currentTxIndex < block->access->chain->txCount()) {
                    currentTxPos = reinterpret_cast<const char *>(block->access->chain->getTx(currentTxIndex));
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
        Block(BlockHeight blockNum_, const DataAccess &access_) : access(&access_), rawBlock(access->chain->getBlock(blockNum_)), blockNum(blockNum_) {
        }
        
        const DataAccess &getAccess() const {
            return *access;
        }
        
        bool operator==(const Block &other) const {
            return *rawBlock == *other.rawBlock;
        }

        bool operator!=(const Block &other) const {
            return !(*rawBlock == *other.rawBlock);
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
        
        uint32_t bits() const {
            return rawBlock->bits;
        }
        
        uint32_t nonce() const {
            return rawBlock->nonce;
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

        Block nextBlock() const;
        Block prevBlock() const;
        
        const std::string getHeaderHash() const;
        
        std::chrono::system_clock::time_point getTime() const;
        
        std::string getString() const;
        
        const std::string coinbaseParam() const {
            auto coinbase = getCoinbase();
            return std::string(coinbase.begin(), coinbase.end());
        }
        
        std::vector<unsigned char> getCoinbase() const;
        Transaction coinbaseTx() const;
    };

    bool isSegwit(const Block &block);
    
    TransactionSummary transactionStatistics(const Block &block, const ChainAccess &access);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, BlockHeight maxAge);
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block);
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block);
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &output);

namespace std {
    template <>
    struct hash<blocksci::Block> {
        typedef blocksci::Block argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return static_cast<size_t>(b.height());
        }
    };
}

#endif /* block_hpp */
