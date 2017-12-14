//
//  blockchain_heuristics.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#ifndef blockchain_heuristics_hpp
#define blockchain_heuristics_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <cstdint>
#include <vector>

namespace blocksci { namespace heuristics {
    std::vector<Transaction> getDeanonTxes(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::vector<Transaction> getChangeOverTxes(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::vector<Transaction> getKeysetChangeTxes(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    
    std::vector<Transaction> getCoinjoinTransactions(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(const Blockchain &chain, uint64_t minBaseFee, double percentageFee, std::size_t maxDepth);
}}

#endif /* blockchain_heuristics_hpp */
