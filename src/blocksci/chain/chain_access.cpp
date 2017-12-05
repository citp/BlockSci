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
        maxHeight = static_cast<BlockHeight>(blockFile.size()) - blocksIgnored;
        if (maxHeight > BlockHeight(0)) {
            const auto &blockFile_ = blockFile;
            auto maxLoadedBlock = blockFile_.getData(static_cast<size_t>(static_cast<int>(maxHeight) - 1));
            lastBlockHash = maxLoadedBlock->hash;
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        }
    }
    
    ChainAccess::ChainAccess(const DataConfiguration &config, bool errorOnReorg_, BlockHeight blocksIgnored_) :
    blockFile(config.blockFilePath()),
    blockCoinbaseFile(config.blockCoinbaseFilePath()),
    txFile(config.txFilePath()),
    txHashesFile(config.txHashesFilePath()),
    blocksIgnored(blocksIgnored_),
    errorOnReorg(errorOnReorg_) {
        setup();
    }
    
    void ChainAccess::reload() {
        blockFile.reload();
        blockCoinbaseFile.reload();
        txFile.reload();
        txHashesFile.reload();
        setup();
    }
    
    size_t ChainAccess::txCount() const {
        return _maxLoadedTx;
    }
    
    BlockHeight ChainAccess::getBlockHeight(uint32_t txIndex) const {
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
    
    std::vector<unsigned char> ChainAccess::getCoinbase(uint64_t offset) const {
        auto pos = blockCoinbaseFile.getDataAtOffset(offset);
        uint64_t length = *reinterpret_cast<const uint32_t *>(pos);
        pos += sizeof(uint32_t);
        auto range = boost::make_iterator_range_n(reinterpret_cast<const unsigned char *>(pos), length);
        return std::vector<unsigned char>(range.begin(), range.end());
    }
    
}
