//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/core_fwd.hpp>
#include <blocksci/core/file_mapper.hpp>
#include <blocksci/core/raw_block.hpp>

#include <blocksci/blocksci_export.h>
#include <blocksci/exception.hpp>
#include <blocksci/typedefs.hpp>

#include <algorithm>

namespace blocksci {
    class BLOCKSCI_EXPORT ChainAccess {
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
        
        void setup();
        
    public:
        explicit ChainAccess(const std::string &baseDirectory, BlockHeight blocksIgnored, bool errorOnReorg);
        
        static std::string txFilePath(const std::string &baseDirectory);
        static std::string txHashesFilePath(const std::string &baseDirectory);
        static std::string blockFilePath(const std::string &baseDirectory);
        static std::string blockCoinbaseFilePath(const std::string &baseDirectory);
        static std::string sequenceFilePath(const std::string &baseDirectory);
        
        uint32_t maxLoadedTx() const {
            return _maxLoadedTx;
        }
        
        BlockHeight getBlockHeight(uint32_t txIndex) const {
            reorgCheck();
            if (errorOnReorg && txIndex >= _maxLoadedTx) {
                throw std::out_of_range("Transaction index out of range");
            }
            auto blockBegin = blockFile[0];
            auto blockEnd = blockFile[static_cast<size_t>(static_cast<int>(maxHeight) - 1)] + 1;
            auto it = std::upper_bound(blockBegin, blockEnd, txIndex, [](uint32_t index, const RawBlock &b) {
                return index < b.firstTxIndex;
            });
            it--;
            return static_cast<BlockHeight>(std::distance(blockBegin, it));
        }
        
        const RawBlock *getBlock(BlockHeight blockHeight) const {
            reorgCheck();
            return blockFile[static_cast<size_t>(blockHeight)];
        }
        
        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile[index];
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
            auto unsignedPos = reinterpret_cast<const unsigned char *>(pos);
            return std::vector<unsigned char>(unsignedPos, unsignedPos + length);
        }
        
        void reload() {
            blockFile.reload();
            blockCoinbaseFile.reload();
            txFile.reload();
            txHashesFile.reload();
            sequenceFile.reload();
            setup();
        }
    };
} // namespace blocksci

#endif /* chain_access_hpp */
