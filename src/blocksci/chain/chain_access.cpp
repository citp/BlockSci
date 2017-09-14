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
    ChainAccess::ChainAccess(const DataConfiguration &config, bool errorOnReorg_, uint32_t blocksIgnored) :
    blockFile(config.blockFilePath()),
    blockCoinbaseFile(config.blockCoinbaseFilePath()),
    txFile(config.txFilePath()),
    txHashesFile(config.txHashesFilePath()),
    errorOnReorg(errorOnReorg_) {

        maxHeight = static_cast<uint32_t>(blockFile.size()) - blocksIgnored;
        if (errorOnReorg_) {
            auto maxLoadedBlock = getBlockFile().getData(maxHeight - 1);
            lastBlockHash = maxLoadedBlock->hash;
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        }
    }
    
    void ChainAccess::reorgCheck() const {
        if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
            throw ReorgException();
        }
    }
    
    uint32_t ChainAccess::maxLoadedTx() const {
        return _maxLoadedTx;
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
        auto linkedTxPos = getTxPos(txIndex);
        linkedTxPos += sizeof(RawTransaction);
        linkedTxPos += sizeof(Output) * outputNum;
        return *reinterpret_cast<const Output *>(linkedTxPos);
    }
    
    const Input &ChainAccess::getInput(uint32_t txIndex, uint16_t inputNum) const {
        auto tx = getTx(txIndex);
        auto txPos = reinterpret_cast<const char *>(tx);
        txPos += sizeof(RawTransaction);
        txPos += sizeof(Output) * tx->outputCount;
        txPos += sizeof(Input) * inputNum;
        return *reinterpret_cast<const Input *>(txPos);
    }
    
    uint32_t ChainAccess::getBlockHeight(uint32_t txIndex) const {
        reorgCheck();
        if (errorOnReorg && txIndex >= _maxLoadedTx) {
            throw std::out_of_range("Transaction index out of range");
        }
        auto blockRange = getBlocks();
        auto it = std::upper_bound(blockRange.begin(), blockRange.end(), txIndex, [](uint32_t txIndex, const Block &b) {
            return txIndex < b.firstTxIndex;
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
    
    
}
