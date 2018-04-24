//
//  chain_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/22/18.
//

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/core/raw_block.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
    ChainAccess::ChainAccess(const std::string &baseDirectory, BlockHeight blocksIgnored, bool errorOnReorg) :
        blockFile(blockFilePath(baseDirectory)),
        blockCoinbaseFile(blockCoinbaseFilePath(baseDirectory)),
        txFile(txFilePath(baseDirectory)),
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
            _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->numTxes;
            lastBlockHashDisk = &maxLoadedBlock->hash;
        } else {
            _maxLoadedTx = 0;
        }
    }
    
    std::string ChainAccess::txFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx").native();
    }
    
    std::string ChainAccess::txHashesFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx_hashes").native();
    }
    
    std::string ChainAccess::blockFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"block").native();
    }
    
    std::string ChainAccess::blockCoinbaseFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"coinbases").native();
    }
    
    std::string ChainAccess::sequenceFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"sequence").native();
    }
} // namespace blocksci
