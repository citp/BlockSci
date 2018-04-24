//
//  blockchain_heuristics.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#ifndef blockchain_heuristics_hpp
#define blockchain_heuristics_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/typedefs.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <cstdint>
#include <vector>

namespace blocksci { namespace heuristics {
    std::vector<Transaction> BLOCKSCI_EXPORT getDeanonTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::vector<Transaction> BLOCKSCI_EXPORT getChangeOverTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::vector<Transaction> BLOCKSCI_EXPORT getKeysetChangeTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    
    std::vector<Transaction> BLOCKSCI_EXPORT getCoinjoinTransactions(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock);
    std::pair<std::vector<Transaction>, std::vector<Transaction>> BLOCKSCI_EXPORT getPossibleCoinjoinTransactions(Blockchain &chain, int64_t minBaseFee, double percentageFee, std::size_t maxDepth);
}}

#endif /* blockchain_heuristics_hpp */
