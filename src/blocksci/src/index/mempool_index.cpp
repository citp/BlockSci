//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include <blocksci/index/mempool_index.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
    
    std::string MempoolIndex::txIndexFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"tx_index").native();
    }
    
    std::string MempoolIndex::blockIndexFilePath(const std::string &baseDirectory) {
        return (boost::filesystem::path{baseDirectory}/"block_index").native();
    }
    
    std::string MempoolIndex::nthTxFilePath(const std::string &baseDirectory, size_t i) {
        return (boost::filesystem::path{baseDirectory}/(std::to_string(i) + "_tx")).native();
    }
    
    std::string MempoolIndex::nthBlockFilePath(const std::string &baseDirectory, size_t i) {
        return (boost::filesystem::path{baseDirectory}/(std::to_string(i) + "_block")).native();
    }
}
