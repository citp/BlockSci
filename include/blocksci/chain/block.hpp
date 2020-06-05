//
//  block.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_hpp
#define block_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/core/raw_block.hpp>

#include <unordered_map>
#include <chrono>
#include <vector>

namespace blocksci {

    /** Represents one Block of the blockchain */
    class BLOCKSCI_EXPORT Block : public TransactionRange {
        /** Contains raw block data from the chain/block.dat file */
        const RawBlock *rawBlock;
        
        Block(const RawBlock *rawBlock_, BlockHeight blockNum_, DataAccess &access_);
        
    public:
        Block() = default;
        Block(const RawBlock *rawBlock_, const Transaction &firstTx) : TransactionRange({rawBlock_->firstTxIndex, rawBlock_->firstTxIndex + rawBlock_->txCount}, firstTx), rawBlock(rawBlock_) {}
        
        Block(BlockHeight blockNum_, DataAccess &access_);
        
        bool operator==(const Block &other) const {
            return *rawBlock == *other.rawBlock;
        }

        bool operator!=(const Block &other) const {
            return !(*rawBlock == *other.rawBlock);
        }
        
        uint256 getHash() const {
            return rawBlock->hash;
        }
        
        BlockHeight height() const {
            return startHeight();
        }
        
        int32_t version() const {
            return rawBlock->version;
        }
        
        uint32_t timestamp() const {
            return rawBlock->timestamp;
        }
        
        // Mempool data
        ranges::optional<std::chrono::system_clock::time_point> getTimeSeen() const;
        ranges::optional<uint32_t> getTimestampSeen() const;
        
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

        Block nextBlock() const {
            return {height() + BlockHeight(1), getAccess()};
        }
        
        Block prevBlock() const {
            return {height() - BlockHeight(1), getAccess()};
        }
        
        std::string getHeaderHash() const;
        
        std::chrono::system_clock::time_point getTime() const {
            return std::chrono::system_clock::from_time_t(static_cast<time_t>(rawBlock->timestamp));
        }
        
        std::string toString() const;
        
        const std::string coinbaseParam() const {
            auto coinbase = getCoinbase();
            return std::string(coinbase.begin(), coinbase.end());
        }
        
        std::vector<unsigned char> getCoinbase() const;
        
        Transaction coinbaseTx() const {
            return (*this)[0];
        }
    };
    
    inline bool BLOCKSCI_EXPORT isSegwit(const Block &block) {
        return isSegwitMarker(block.coinbaseTx());
    }
    
    std::unordered_map<AddressType::Enum, int64_t> BLOCKSCI_EXPORT netAddressTypeValue(const Block &block);
    
    std::unordered_map<std::string, int64_t> BLOCKSCI_EXPORT netFullTypeValue(const Block &block);
    
    std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const blocksci::Block &block);
} // namespace blocksci



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
