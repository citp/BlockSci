//
//  change_address.hpp
//  blocksci
//
//  Created by Malte Möser on 10/2/17.
//

#ifndef change_address_hpp
#define change_address_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <range/v3/utility/optional.hpp>

#include <unordered_set>

namespace blocksci {
namespace heuristics {
    
    bool isPeelingChain(const Transaction &tx);
    
    // If tx is a peeling chain, returns the smaller output.
    std::unordered_set<Output> changeByPeelingChain(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByPeelingChain(const Transaction &tx);
    
    // Detects a change output by checking for output values that are multiples of 10^digits.
    std::unordered_set<Output> changeByPowerOfTenValue(const Transaction &tx, int digits = 6);
    ranges::optional<Output> uniqueChangeByPowerOfTenValue(const Transaction &tx, int digits = 6);
    
    std::unordered_set<Output> changeByOptimalChange(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByOptimalChange(const Transaction &tx);
    
    std::unordered_set<Output> changeByDedupAddressType(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByDedupAddressType(const Transaction &tx);
    
    std::unordered_set<Output> changeByLocktime(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByLocktime(const Transaction &tx);
    
    std::unordered_set<Output> changeByAddressReuse(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByAddressReuse(const Transaction &tx);
    
    std::unordered_set<Output> changeByClientChangeAddressBehavior(const Transaction &tx, const ScriptAccess &scripts);
    ranges::optional<Output> uniqueChangeByClientChangeAddressBehavior(const Transaction &tx, const ScriptAccess &scripts);
    
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx, const ScriptAccess &scripts);
    
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    
    std::unordered_set<Output> changeByClientChangeAddressBehavior(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByClientChangeAddressBehavior(const Transaction &tx);
    
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx);
    #endif
}}

#endif /* change_address_hpp */
