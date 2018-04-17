//
//  tx_identification.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#ifndef tx_identification_hpp
#define tx_identification_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <stdio.h>

namespace blocksci {
    class DataAccess;
    namespace heuristics {
    
    enum class BLOCKSCI_EXPORT CoinJoinResult {
        True, False, Timeout
    };
    
    bool BLOCKSCI_EXPORT isCoinjoin(const Transaction &tx);
    CoinJoinResult BLOCKSCI_EXPORT isPossibleCoinjoin(const Transaction &tx, int64_t minBaseFee, double percentageFee, size_t maxDepth);
    CoinJoinResult BLOCKSCI_EXPORT isCoinjoinExtra(const Transaction &tx, int64_t minBaseFee, double percentageFee, size_t maxDepth);
    bool BLOCKSCI_EXPORT isDeanonTx(const Transaction &tx);
    bool BLOCKSCI_EXPORT containsKeysetChange(const Transaction &tx);
    bool BLOCKSCI_EXPORT isChangeOverTx(const Transaction &tx);
}}


#endif /* tx_identification_hpp */
