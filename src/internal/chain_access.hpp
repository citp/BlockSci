//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include "file_mapper.hpp"
#include "exception.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/core_fwd.hpp>
#include <blocksci/core/raw_block.hpp>
#include <blocksci/core/raw_transaction.hpp>
#include <blocksci/core/typedefs.hpp>
#include <blocksci/core/transaction_data.hpp>

#include <wjfilesystem/path.h>

#include <algorithm>

namespace blocksci {
    
    class ChainAccess {
        FixedSizeFileMapper<RawBlock> blockFile;
        SimpleFileMapper<> blockCoinbaseFile;
        
        IndexedFileMapper<mio::access_mode::read, RawTransaction> txFile;
        FixedSizeFileMapper<int32_t> txVersionFile;
        FixedSizeFileMapper<uint64_t> txFirstInputFile;
        FixedSizeFileMapper<uint64_t> txFirstOutputFile;
        FixedSizeFileMapper<uint32_t> sequenceFile;
        
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
            if (blocksIgnored <= 0) {
                maxHeight = static_cast<BlockHeight>(blockFile.size()) + blocksIgnored;
            } else {
                maxHeight = blocksIgnored;
            }
            if (maxHeight > BlockHeight(0)) {
                auto maxLoadedBlock = blockFile[static_cast<OffsetType>(maxHeight) - 1];
                lastBlockHash = maxLoadedBlock->hash;
                _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->txCount;
                lastBlockHashDisk = &maxLoadedBlock->hash;
            } else {
                lastBlockHash.SetNull();
                _maxLoadedTx = 0;
                lastBlockHashDisk = nullptr;
            }
        }
        
    public:
        explicit ChainAccess(const filesystem::path &baseDirectory, BlockHeight blocksIgnored, bool errorOnReorg) :
        blockFile(blockFilePath(baseDirectory)),
        blockCoinbaseFile(blockCoinbaseFilePath(baseDirectory)),
        txFile(txFilePath(baseDirectory)),
        txVersionFile(txVersionFilePath(baseDirectory)),
        txFirstInputFile(firstInputFilePath(baseDirectory)),
        txFirstOutputFile(firstOutputFilePath(baseDirectory)),
        sequenceFile(sequenceFilePath(baseDirectory)),
        txHashesFile(txHashesFilePath(baseDirectory)),
        blocksIgnored(blocksIgnored),
        errorOnReorg(errorOnReorg) {
            setup();
        }
        
        static filesystem::path txFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx";
        }
        
        static filesystem::path txHashesFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx_hashes";
        }
        
        static filesystem::path blockFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"block";
        }
        
        static filesystem::path blockCoinbaseFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"coinbases";
        }
        
        static filesystem::path txVersionFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx_version";
        }
        
        static filesystem::path firstInputFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"firstInput";
        }
        
        static filesystem::path firstOutputFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"firstOutput";
        }
        
        static filesystem::path sequenceFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"sequence";
        }
        
        static filesystem::path inputSpentOutNumFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"input_out_num";
        }
        
        static filesystem::path outputSpendingInputNumFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"output_in_num";
        }
        
        BlockHeight getBlockHeight(uint32_t txIndex) const {
            reorgCheck();
            if (errorOnReorg && txIndex >= _maxLoadedTx) {
                throw std::out_of_range("Transaction index out of range");
            }
            auto blockBegin = blockFile[0];
            auto blockEnd = blockFile[static_cast<OffsetType>(maxHeight) - 1] + 1;
            auto it = std::upper_bound(blockBegin, blockEnd, txIndex, [](uint32_t index, const RawBlock &b) {
                return index < b.firstTxIndex;
            });
            it--;
            return static_cast<BlockHeight>(std::distance(blockBegin, it));
        }
        
        const RawBlock *getBlock(BlockHeight blockHeight) const {
            reorgCheck();
            return blockFile[static_cast<OffsetType>(blockHeight)];
        }
        
        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile[index];
        }
        
        const RawTransaction *getTx(uint32_t index) const {
            reorgCheck();
            return txFile.getData(index);
        }
        
        const int32_t *getTxVersion(uint32_t index) const {
            reorgCheck();
            return txVersionFile[index];
        }
        
        const uint32_t *getSequenceNumbers(uint32_t index) const {
            reorgCheck();
            return sequenceFile[static_cast<OffsetType>(*txFirstInputFile[index])];
        }
        
        TxData getTxData(uint32_t index) const {
            reorgCheck();
            return {
                txFile.getData(index),
                nullptr, // txVersionFile[index]
                txHashesFile[index],
                sequenceFile[static_cast<OffsetType>(*txFirstInputFile[index])]
            };
        }
        
        size_t txCount() const {
            return _maxLoadedTx;
        }
        
        uint64_t inputCount() const {
            if (_maxLoadedTx > 0) {
                auto lastTx = getTx(_maxLoadedTx - 1);
                return *txFirstInputFile[_maxLoadedTx - 1] + lastTx->inputCount;
            } else {
                return 0;
            }
        }
        
        uint64_t outputCount() const {
            if (_maxLoadedTx > 0) {
                auto lastTx = getTx(_maxLoadedTx);
                return *txFirstOutputFile[_maxLoadedTx - 1] + lastTx->outputCount;
            } else {
                return 0;
            }
        }
        
        BlockHeight blockCount() const {
            return maxHeight;
        }
        
        std::vector<unsigned char> getCoinbase(uint64_t offset) const {
            auto pos = blockCoinbaseFile.getDataAtOffset(static_cast<OffsetType>(offset));
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
