//
//  tx_identification.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#ifndef tx_identification_hpp
#define tx_identification_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <stdio.h>

namespace blocksci {
    class DataAccess;
    namespace heuristics {
    
    enum class CoinJoinResult {
        True, False, Timeout
    };
    
    bool isCoinjoin(const Transaction &tx);
    CoinJoinResult isPossibleCoinjoin(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    CoinJoinResult isCoinjoinExtra(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    bool isDeanonTx(const Transaction &tx);
    bool containsKeysetChange(const Transaction &tx);
    bool isChangeOverTx(const Transaction &tx);
}}


#endif /* tx_identification_hpp */
