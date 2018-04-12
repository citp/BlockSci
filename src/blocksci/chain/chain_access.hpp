//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include <blocksci/blocksci_export.h>
#include "chain_fwd.hpp"

#include "raw_block.hpp"
#include "raw_transaction.hpp"

#include <blocksci/util/file_mapper.hpp>
#include <blocksci/util/data_configuration.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <memory>

namespace blocksci {
    
    class BLOCKSCI_EXPORT ReorgException : public std::runtime_error {
    public:
        ReorgException() : std::runtime_error("Blockchain has experienced reorg") {}
    };
    
    struct DataConfiguration;
    
    class ChainAccess {
        FixedSizeFileMapper<RawBlock> blockFile;
        SimpleFileMapper<> blockCoinbaseFile;
        
        IndexedFileMapper<AccessMode::readonly, RawTransaction> txFile;
        IndexedFileMapper<AccessMode::readonly, uint32_t> sequenceFile;
        
        FixedSizeFileMapper<uint256> txHashesFile;
        
        uint256 lastBlockHash;
        const uint256 *lastBlockHashDisk = nullptr;
        BlockHeight maxHeight = 0;
        uint32_t _maxLoadedTx = 0;
        BlockHeight blocksIgnored = 0;
        bool errorOnReorg = false;
        
        void reorgCheck() const {
            if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
                throw ReorgException();
            }
        }
        
        void setup() {
            maxHeight = static_cast<BlockHeight>(blockFile.size()) - blocksIgnored;
            if (maxHeight > BlockHeight(0)) {
                const auto &blockFile_ = blockFile;
                auto maxLoadedBlock = blockFile_.getData(static_cast<size_t>(static_cast<int>(maxHeight) - 1));
                lastBlockHash = maxLoadedBlock->hash;
                _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
                lastBlockHashDisk = &maxLoadedBlock->hash;
            } else {
                _maxLoadedTx = 0;
            }
        }
        
    public:
        explicit ChainAccess(const DataConfiguration &config) :
        blockFile(config.blockFilePath()),
        blockCoinbaseFile(config.blockCoinbaseFilePath()),
        txFile(config.txFilePath()),
        sequenceFile(config.sequenceFilePath()),
        txHashesFile(config.txHashesFilePath()),
        blocksIgnored(config.blocksIgnored),
        errorOnReorg(config.errorOnReorg) {
            setup();
        }
        
        uint32_t maxLoadedTx() const {
            return _maxLoadedTx;
        }
        
        BlockHeight getBlockHeight(uint32_t txIndex) const {
            reorgCheck();
            if (errorOnReorg && txIndex >= _maxLoadedTx) {
                throw std::out_of_range("Transaction index out of range");
            }
            auto blockRange = ranges::make_iterator_range(blockFile.getData(0), blockFile.getData(static_cast<size_t>(static_cast<int>(maxHeight) - 1)) + 1);
            auto it = std::upper_bound(blockRange.begin(), blockRange.end(), txIndex, [](uint32_t index, const RawBlock &b) {
                return index < b.firstTxIndex;
            });
            it--;
            return static_cast<BlockHeight>(std::distance(blockRange.begin(), it));
        }
        
        const RawBlock *getBlock(BlockHeight blockHeight) const {
            reorgCheck();
            return blockFile.getData(static_cast<size_t>(static_cast<int>(blockHeight)));
        }
        
        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile.getData(index);
        }
        
        const RawTransaction *getTx(uint32_t index) const {
            return txFile.getData(index);
        }
        
        const uint32_t *getSequenceNumbers(uint32_t index) const {
            return sequenceFile.getData(index);
        }
        
        size_t txCount() const {
            return _maxLoadedTx;
        }
        
        BlockHeight blockCount() const {
            return maxHeight;
        }
        
        std::vector<unsigned char> getCoinbase(uint64_t offset) const {
            auto pos = blockCoinbaseFile.getDataAtOffset(offset);
            uint32_t coinbaseLength;
            std::memcpy(&coinbaseLength, pos, sizeof(coinbaseLength));
            uint64_t length = coinbaseLength;
            pos += sizeof(coinbaseLength);
            auto range = boost::make_iterator_range_n(reinterpret_cast<const unsigned char *>(pos), length);
            return std::vector<unsigned char>(range.begin(), range.end());
        }
        
        void reload() {
            blockFile.reload();
            blockCoinbaseFile.reload();
            txFile.reload();
            txHashesFile.reload();
            setup();
        }
    };
} // namespace blocksci

#endif /* chain_access_hpp */
