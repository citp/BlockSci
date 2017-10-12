//
//  chain_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "chain_access.hpp"
#include "data_configuration.hpp"
#include "block.hpp"
#include "output_pointer.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"
#include "transaction_iterator.hpp"

namespace blocksci {
    
    ReorgException::ReorgException() : std::runtime_error("") {}
    
    ReorgException::~ReorgException() = default;
    
    void ChainAccess::setup() {
        maxHeight = static_cast<uint32_t>(blockFile.size()) - blocksIgnored;
        if (errorOnReorg) {
            auto maxLoadedBlock = getBlockFile().getData(maxHeight - 1);
            lastBlockHash = maxLoadedBlock->hash;
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        }
    }
    
    ChainAccess::ChainAccess(const DataConfiguration &config, bool errorOnReorg_, uint32_t blocksIgnored_) :
    blockFile(config.blockFilePath()),
    blockCoinbaseFile(config.blockCoinbaseFilePath()),
    txFile(config.txFilePath()),
    txHashesFile(config.txHashesFilePath()),
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
        blockFile.reload();
        blockCoinbaseFile.reload();
        txFile.reload();
        txHashesFile.reload();
        setup();
    }
    
    size_t ChainAccess::txCount() const {
        reorgCheck();
        return txFile.size();
    }
    
    const char *ChainAccess::getTxPos(uint32_t index) const {
        reorgCheck();
        return txFile.getPointerAtIndex(index);
    }
    
    const RawTransaction *ChainAccess::getTx(uint32_t index) const {
        return txFile.getData(index);
    }
    
    const Output &ChainAccess::getOutput(uint32_t txIndex, uint16_t outputNum) const {
        return getTx(txIndex)->getOutput(outputNum);
    }
    
    const Input &ChainAccess::getInput(uint32_t txIndex, uint16_t inputNum) const {
        return getTx(txIndex)->getInput(inputNum);
    }
    
    uint32_t ChainAccess::getBlockHeight(uint32_t txIndex) const {
        reorgCheck();
        if (errorOnReorg && txIndex >= _maxLoadedTx) {
            throw std::out_of_range("Transaction index out of range");
        }
        auto blockRange = getBlocks();
        auto it = std::upper_bound(blockRange.begin(), blockRange.end(), txIndex, [](uint32_t index, const Block &b) {
            return index < b.firstTxIndex;
        });
        it--;
        auto height = static_cast<uint32_t>(std::distance(blockRange.begin(), it));
        return height;
    }
    
    const Block &ChainAccess::getBlock(uint32_t blockHeight) const {
        reorgCheck();
        return *blockFile.getData(blockHeight);
    }
    
    const boost::iterator_range<const Block *> ChainAccess::getBlocks() const {
        reorgCheck();
        if (blockFile.size() > 0) {
            auto fullRange = blockFile.getRange();
            return boost::iterator_range<const Block *>(fullRange.begin(), fullRange.begin() + maxHeight);
        } else {
            return boost::iterator_range<const Block *>{};
        }
    }
    
    std::vector<unsigned char> ChainAccess::getCoinbase(uint64_t offset) const {
        auto pos = blockCoinbaseFile.getDataAtOffset(offset);
        uint64_t length = *reinterpret_cast<const uint32_t *>(pos);
        pos += sizeof(uint32_t);
        auto range = boost::make_iterator_range_n(reinterpret_cast<const unsigned char *>(pos), length);
        return std::vector<unsigned char>(range.begin(), range.end());
    }
    
    boost::iterator_range<TransactionIterator> iterateTransactions(const ChainAccess &chain, uint32_t startIndex, uint32_t endIndex) {
        auto begin = TransactionIterator(&chain, startIndex);
        auto end = TransactionIterator(&chain, endIndex);
        return boost::make_iterator_range(begin, end);
    }
    
    boost::iterator_range<TransactionIterator> iterateTransactions(const ChainAccess &chain, const Block &beginBlock, const Block &endBlock) {
        auto begin = TransactionIterator(&chain, beginBlock.firstTxIndex, beginBlock.height);
        auto end = TransactionIterator(&chain, endBlock.firstTxIndex + endBlock.numTxes, endBlock.height + 1);
        return boost::make_iterator_range(begin, end);
    }
    
}
