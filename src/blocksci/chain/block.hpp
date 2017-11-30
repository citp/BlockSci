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
#include <blocksci/chain/chain_access.hpp>

#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/address/address_types.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>

#include <unordered_map>
#include <chrono>

namespace blocksci {
    class Block : public ranges::view_facade<Block> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        const RawBlock *rawBlock;
        uint32_t blockNum;
        
        struct cursor {
        private:
            const Block *block;
            const char *currentTxPos;
            uint32_t currentTxIndex;
        public:
            cursor() = default;
            cursor(const Block &block_, uint32_t txNum) : block(&block_), currentTxPos(reinterpret_cast<const char *>(block->access->getTx(txNum))), currentTxIndex(txNum) {}
            
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
                return static_cast<int>(currentTxIndex) - static_cast<int>(that.currentTxIndex);
            }
            
            int distance_to(ranges::default_sentinel) const {
                return static_cast<int>(block->endTxIndex()) - static_cast<int>(currentTxIndex);
            }
            
            void prev() {
                currentTxIndex--;
                currentTxPos = reinterpret_cast<const char *>(block->access->getTx(currentTxIndex));
            }
            
            void advance(int amount) {
                currentTxIndex += static_cast<uint32_t>(amount);
                currentTxPos = reinterpret_cast<const char *>(block->access->getTx(currentTxIndex));
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
        Block(uint32_t blockNum_, const ChainAccess &access_) : access(&access_), rawBlock(access->getBlock(blockNum_)), blockNum(blockNum_) {
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
        
        uint32_t height() const {
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
        
        const std::string getHeaderHash() const;
        
        std::chrono::system_clock::time_point getTime() const;
        
        std::string getString() const;
        
        const std::string coinbaseParam() const {
            auto coinbase = getCoinbase();
            return std::string(coinbase.begin(), coinbase.end());
        }
        
        std::vector<unsigned char> getCoinbase() const;
        Transaction coinbaseTx() const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Block(uint32_t blockNum);
        #endif
    };
    
    inline auto txes(const Block &block) {
        return block;
    }
    
    size_t sizeBytes(const Block &block);
    bool isSegwit(const Block &block, const ScriptAccess &scripts);
    
    TransactionSummary transactionStatistics(const Block &block, const ChainAccess &access);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, const ChainAccess &access, uint32_t maxAge);
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block);
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block, const ScriptAccess &scripts);

    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    bool isSegwit(const Block &block);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, uint32_t maxAge);
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block);
    #endif
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &output);

#endif /* block_hpp */
