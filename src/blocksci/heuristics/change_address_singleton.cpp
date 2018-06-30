//
//  change_address_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 12/4/17.
//

#include "change_address.hpp"
#include "util/data_access.hpp"

#include "chain/output.hpp"

namespace blocksci { namespace heuristics {
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx) {
        return uniqueChangeByLegacyHeuristic(tx, *DataAccess::Instance().scripts);
    }
    
    std::unordered_set<Output> changeByClientChangeAddressBehavior(const Transaction &tx) {
        return changeByClientChangeAddressBehavior(tx, *DataAccess::Instance().scripts);
    }
    
    ranges::optional<Output> uniqueChangeByClientChangeAddressBehavior(const Transaction &tx) {
        return uniqueChangeByClientChangeAddressBehavior(tx, *DataAccess::Instance().scripts);
    }
}}
