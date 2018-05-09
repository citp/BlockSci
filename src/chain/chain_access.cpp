//
//  chain_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/22/18.
//

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/core/raw_transaction.hpp>
#include <blocksci/core/raw_block.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
    ChainAccess::ChainAccess(const std::string &baseDirectory, BlockHeight blocksIgnored, bool errorOnReorg) :
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
    
    void ChainAccess::setup() {
        maxHeight = static_cast<BlockHeight>(blockFile.size()) - blocksIgnored;
        if (maxHeight > BlockHeight(0)) {
            const auto &blockFile_ = blockFile;
            auto maxLoadedBlock = blockFile_[static_cast<size_t>(static_cast<int>(maxHeight) - 1)];
            lastBlockHash = maxLoadedBlock->hash;
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->txCount;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        } else {
            lastBlockHash.SetNull();
            _maxLoadedTx = 0;
            lastBlockHashDisk = nullptr;
        }
    }
    
    uint64_t ChainAccess::inputCount() const {
        if (_maxLoadedTx > 0) {
            auto lastTx = getTx(_maxLoadedTx - 1);
            return *txFirstInputFile[_maxLoadedTx - 1] + lastTx->inputCount;
        } else {
            return 0;
        }
    }
    
    uint64_t ChainAccess::outputCount() const {
        if (_maxLoadedTx > 0) {
            auto lastTx = getTx(_maxLoadedTx);
            return *txFirstOutputFile[_maxLoadedTx - 1] + lastTx->outputCount;
        } else {
            return 0;
        }
    }
    
    std::string ChainAccess::txFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx").native();
    }
    
    std::string ChainAccess::txHashesFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx_hashes").native();
    }
    
    std::string ChainAccess::txVersionFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx_version").native();
    }
    
    std::string ChainAccess::blockFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"block").native();
    }
    
    std::string ChainAccess::blockCoinbaseFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"coinbases").native();
    }
    
    std::string ChainAccess::firstInputFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"firstInput").native();
    }
    
    std::string ChainAccess::firstOutputFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"firstOutput").native();
    }
    
    std::string ChainAccess::sequenceFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"sequence").native();
    }
    
    std::string ChainAccess::inputSpentOutNumFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"input_out_num").native();
    }
    
    std::string ChainAccess::outputSpendingInputNumFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"output_in_num").native();
    }
    
    
} // namespace blocksci
