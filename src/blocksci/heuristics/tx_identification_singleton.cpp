//
//  tx_identification_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#include "tx_identification.hpp"
#include "util/data_access.hpp"

namespace blocksci { namespace heuristics {
    bool isChangeOverTx(const Transaction &tx) {
        return isChangeOverTx(tx, *DataAccess::Instance().scripts);
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        return containsKeysetChange(tx, *DataAccess::Instance().scripts);
    }
}}
