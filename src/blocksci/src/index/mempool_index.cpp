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
    
    std::string MempoolIndex::txIndexFilePath() const {
        return (boost::filesystem::path{config.mempoolDirectory()}/"tx_index").native();
    }
    
    std::string MempoolIndex::blockIndexFilePath() const {
        return (boost::filesystem::path{config.mempoolDirectory()}/"block_index").native();
    }
    
    std::string MempoolIndex::nthTxFilePath(size_t i) const {
        return (boost::filesystem::path{config.mempoolDirectory()}/(std::to_string(i) + "_tx")).native();
    }
    
    std::string MempoolIndex::nthBlockFilePath(size_t i) const {
        return (boost::filesystem::path{config.mempoolDirectory()}/(std::to_string(i) + "_block")).native();
    }
}
