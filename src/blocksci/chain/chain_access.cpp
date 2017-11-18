//
//  chain_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "chain_access.hpp"
#include "util/data_configuration.hpp"
#include "block.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"

#include <blocksci/util/file_mapper.hpp>

#include <range/v3/iterator_range.hpp>

namespace blocksci {
    
    ReorgException::ReorgException() : std::runtime_error("") {}
    
    ReorgException::~ReorgException() = default;
    
    void ChainAccess::setup() {
        const auto &blockFile_ = *blockFile;
        maxHeight = static_cast<uint32_t>(blockFile_.size()) - blocksIgnored;
        if (maxHeight > 0) {
            auto maxLoadedBlock = blockFile_.getData(maxHeight - 1);
            lastBlockHash = maxLoadedBlock->hash;
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        }
    }
    
    ChainAccess::ChainAccess(const DataConfiguration &config, bool errorOnReorg_, uint32_t blocksIgnored_) :
    blockFile(std::make_unique<FixedSizeFileMapper<RawBlock>>(config.blockFilePath())),
    blockCoinbaseFile(std::make_unique<SimpleFileMapper<>>(config.blockCoinbaseFilePath())),
    txFile(std::make_unique<IndexedFileMapper<AccessMode::readonly, RawTransaction>>(config.txFilePath())),
    txHashesFile(std::make_unique<FixedSizeFileMapper<uint256>>(config.txHashesFilePath())),
    blocksIgnored(blocksIgnored_),
    errorOnReorg(errorOnReorg_) {
        setup();
    }
    
    void ChainAccess::reorgCheck() const {
        if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
            throw ReorgException();
        }
    }
    
    uint32_t ChainAccess::maxLoadedTx() const {
        return _maxLoadedTx;
    }
    
    void ChainAccess::reload() {
        blockFile->reload();
        blockCoinbaseFile->reload();
        txFile->reload();
        txHashesFile->reload();
        setup();
    }
    
    size_t ChainAccess::txCount() const {
        return _maxLoadedTx;
    }
    
    size_t ChainAccess::blockCount() const {
        return maxHeight;
    }
    
    const RawTransaction *ChainAccess::getTx(uint32_t index) const {
        const auto &txFile_ = *txFile;
        return txFile_.getData(index);
    }
    
    uint32_t ChainAccess::getBlockHeight(uint32_t txIndex) const {
        reorgCheck();
        if (errorOnReorg && txIndex >= _maxLoadedTx) {
            throw std::out_of_range("Transaction index out of range");
        }
        const auto &blockFile_ = *blockFile;
        auto blockRange = ranges::make_iterator_range(blockFile_.getData(0), blockFile_.getData(maxHeight - 1) + 1);
        auto it = std::upper_bound(blockRange.begin(), blockRange.end(), txIndex, [](uint32_t index, const RawBlock &b) {
            return index < b.firstTxIndex;
        });
        it--;
        auto height = static_cast<uint32_t>(std::distance(blockRange.begin(), it));
        return height;
    }
    
    const RawBlock *ChainAccess::getBlock(uint32_t blockHeight) const {
        reorgCheck();
        const auto &blockFile_ = *blockFile;
        return blockFile_.getData(blockHeight);
    }

    const uint256 *ChainAccess::getTxHash(uint32_t index) const {
        reorgCheck();
        const auto &txHashesFile_ = *txHashesFile;
        return txHashesFile_.getData(index);
    }
    
    
    std::vector<unsigned char> ChainAccess::getCoinbase(uint64_t offset) const {
        auto pos = blockCoinbaseFile->getDataAtOffset(offset);
        uint64_t length = *reinterpret_cast<const uint32_t *>(pos);
        pos += sizeof(uint32_t);
        auto range = boost::make_iterator_range_n(reinterpret_cast<const unsigned char *>(pos), length);
        return std::vector<unsigned char>(range.begin(), range.end());
    }
    
}
