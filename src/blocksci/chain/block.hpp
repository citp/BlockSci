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

#include <blocksci/bitcoin_uint256.hpp>
#include <blocksci/address/address_types.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>

#include <unordered_map>
#include <chrono>

namespace blocksci {
    
    class ScriptAccess;
    
    struct RawBlock {
        uint32_t firstTxIndex;
        uint32_t numTxes;
        uint32_t height;
        uint256 hash;
        int32_t version;
        uint32_t timestamp;
        uint32_t bits;
        uint32_t nonce;
        uint64_t coinbaseOffset;
        
        RawBlock(uint32_t firstTxIndex, uint32_t numTxes, uint32_t height, uint256 hash, int32_t version, uint32_t timestamp, uint32_t bits, uint32_t nonce, uint64_t coinbaseOffset);
        
        bool operator==(const RawBlock& other) const;
    };
    
    class Block : public ranges::view_facade<Block> {
        friend ranges::range_access;
        
        const ChainAccess *access;
        const RawBlock *rawBlock;
        uint32_t blockNum;
        
        struct cursor {
        private:
            const Block *block;
            mutable const char *currentTxPos;
            uint32_t currentTxIndex;
        public:
            cursor() = default;
            cursor(const Block &block_, uint32_t txNum) : block(&block_), currentTxPos(nullptr), currentTxIndex(txNum) {}
            Transaction read() const;
            bool equal(cursor const &that) const;
            bool equal(ranges::default_sentinel) const;
            void next();
            void prev();
            int distance_to(cursor const &that) const;
            void advance(int amount);
        };
        
        cursor begin_cursor() const {
            return cursor(*this, firstTxIndex());
        }
        
        cursor end_cursor() const {
            return cursor(*this, endTxIndex());
        }
        
    public:
        Block() = default;
        Block(uint32_t blockNum, const ChainAccess &access);
        
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
    
    std::vector<uint64_t> fees(const Block &block);
    std::vector<double> feesPerByte(const Block &block);
    uint64_t totalOut(const Block &block);
    uint64_t totalIn(const Block &block);
    
    
    size_t sizeBytes(const Block &block);
    bool isSegwit(const Block &block, const ScriptAccess &scripts);
    
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height);
    std::vector<Output> getUnspentOutputs(const Block &block);
    
    TransactionSummary transactionStatistics(const Block &block, const ChainAccess &access);
    std::vector<Output> getOutputsSpentByHeight(const Block &block, uint32_t height, const ChainAccess &access);
    uint64_t getTotalSpentOfAge(const Block &block, const ChainAccess &access, uint32_t age);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, const ChainAccess &access, uint32_t maxAge);
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block);
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block, const ScriptAccess &scripts);

    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    bool isSegwit(const Block &block);
    std::vector<Output> getOutputsSpentByHeight(const Block &block, uint32_t height);
    uint64_t getTotalSpentOfAge(const Block &block, uint32_t age);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, uint32_t maxAge);
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block);
    #endif
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &output);

#endif /* block_hpp */
