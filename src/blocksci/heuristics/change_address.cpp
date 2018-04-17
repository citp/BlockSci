//
//  change_address.cpp
//  blocksci
//
//  Created by Malte Möser on 10/2/17.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "change_address.hpp"
#include "tx_identification.hpp"
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <unordered_set>
#include <cmath>


// Common change address heuristics
// Every heuristic returns the set of outputs it cannot rule out as change

namespace blocksci { namespace heuristics {
    
    // Takes an unordered set of outputs as input.
    // If there is exactly one output in the set, returns it.
    // Otherwise returns a nullptr.
    namespace internal {
        ranges::optional<Output> singleOrNullptr(std::unordered_set<Output> candidates) {
            if(candidates.size() == 1) {
                return *candidates.begin();
            } else {
                return ranges::nullopt;
            }
        }
    }
    
    // Remove OP_RETURN outputs from candidate set
    std::unordered_set<Output> removeOpReturnOutputs(std::unordered_set<Output> candidates) {
        std::unordered_set<Output> filtered_candidates;
        for(auto output : candidates) {
            if(output.getAddress().isSpendable()) {
                filtered_candidates.insert(output);
            }
        }
        return filtered_candidates;
    }
    
    // Peeling chains have one input and two outputs
    bool looksLikePeelingChain(const Transaction &tx) {
        return (tx.outputCount() == 2 && tx.inputCount() == 1);
    }
    
    // A transaction is considered a peeling chain if it has one input and two outputs,
    // and either the previous or one of the next transactions looks like a peeling chain.
    bool isPeelingChain(const Transaction &tx) {
        if(!looksLikePeelingChain(tx)) {
            return false;
        }
        // Check if past transaction is peeling chain
        if(looksLikePeelingChain(tx.inputs()[0].getSpentTx())) {
            return true;
        }
        // Check if future transaction is peeling chain
        for (auto output : tx.outputs()) {
            if(output.isSpent() && looksLikePeelingChain(*output.getSpendingTx())) {
                return true;
            }
        }
        return false;
    }
    
    // Peeling chains 'peel off' small amounts of bitcoins in every transaction,
    // using the change as the input to the next peeling chain.
    // Thus, the change is usually the larger output.
    // TODO: check if it looks like peeling chain?
    ranges::optional<Output> uniqueChangeByPeelingChain(const Transaction &tx) {
        if(isPeelingChain(tx)) {
            if(tx.outputs()[0].getValue() > tx.outputs()[1].getValue()) {
                return tx.outputs()[0];
            } else {
                return tx.outputs()[1];
            }
        }
        return ranges::nullopt;
    }
    
    // This function mostly exists to ensure a consistent API.
    // The set it returns will never contain more than one output.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::PeelingChain>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        auto candidate = uniqueChangeByPeelingChain(tx);
        if(candidate) {
            candidates.insert(*candidate);
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // When users transfer bitcoins between wallets, they often do so with values that are powers of ten.
    // On the other hand, it is extremly unlikely that you receive power of ten change due to a wallet's coin selection.
    // Default for digits is 6 (i.e. 0.01 BTC)
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::PowerOfTen>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        int64_t value = 1;
        for (int i = 0; i < digits; ++i)
        {
            value *= 10;
        }
        for(auto output : tx.outputs()) {
            if(output.getValue() % value != 0) {
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // If there exists an output that is smaller than any of the inputs it is likely the change.
    // If a change output was larger than the smallest input, then the coin selection algorithm
    // wouldn't need to add the input in the first place.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::OptimalChange>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        auto smallestInputValue = tx.inputs()[0].getValue();
        for(auto input : tx.inputs()) {
            smallestInputValue = std::min(smallestInputValue, input.getValue());
        }
        
        for(Output output : tx.outputs()) {
            if(output.getValue() < smallestInputValue) {
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // If all inputs are of one address type (e.g., P2PKH or P2SH),
    // it is likely that the change output has the same type
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::AddressType>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        // check whether all inputs have the same type (e.g., P2SH)
        bool allInputsSameType = true;
        AddressType::Enum inputType = tx.inputs()[0].getType();
        for(auto input : tx.inputs()) {
            if(input.getType() != inputType) {
                allInputsSameType = false;
                break;
            }
        }
        if(allInputsSameType) {
            for(auto output : tx.outputs()) {
                if(output.getType() == inputType) {
                    candidates.insert(output);
                }
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // Bitcoin Core sets the locktime to the current block height to prevent fee sniping.
    // If all outpus have been spent, and there is only one output that has been spent
    // in a transaction that matches this transaction's locktime behavior, it is the change.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::Locktime>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        // locktime of tx
        bool locktimeGreaterZero = tx.locktime() > 0;
        
        for(auto output : tx.outputs()) {
            // output has been spent, check if locktime is consistent
            if(output.isSpent()) {
                Transaction nextTx = *output.getSpendingTx();
                if((nextTx.locktime() > 0) == locktimeGreaterZero) {
                    candidates.insert(output);
                }
            } else { // not spent, hence unknown
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // If input addresses appear as an output address,
    // the client might have reused addresses for change.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::AddressReuse>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        std::unordered_set<Address> inputAddresses;
        for(auto input : tx.inputs()) {
            inputAddresses.insert(input.getAddress());
        }
        
        for(auto output : tx.outputs()) {
            if(inputAddresses.find(output.getAddress()) != inputAddresses.end()) {
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
   
    // Most clients will generate a fresh address for the change.
    // If an output is the first to send value to an address, it is potentially the change.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        for (auto output : tx.outputs()) {
            if (output.getAddress().isSpendable() && output.getAddress().getScript().firstTxIndex() == tx.txNum) {
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // Legacy heuristic used in previous versions of BlockSci
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx) {
        if (isCoinjoin(tx)) {
            return ranges::nullopt;
        }
        
        auto smallestInput = std::numeric_limits<int64_t>::max();
        for (auto input : tx.inputs()) {
            smallestInput = std::min(smallestInput, input.getValue());
        }
        
        uint16_t spendableCount = 0;
        ranges::optional<Output> change;
        for (auto output : tx.outputs()) {
            if (output.getAddress().isSpendable()) {
                spendableCount++;
                if (output.getValue() < smallestInput && output.getAddress().getScript().firstTxIndex() == tx.txNum) {
                    if (change) {
                        return ranges::nullopt;
                    }
                    change = output;
                }
            }
        }
        if (change && spendableCount > 1) {
            return change;
        } else {
            return ranges::nullopt;
        }
    }
    
    // This function mostly exists to ensure a consistent API.
    // The set it returns will never contain more than one output.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::Legacy>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        auto candidate = uniqueChangeByLegacyHeuristic(tx);
        if(candidate) {
            candidates.insert(*candidate);
        }
        return removeOpReturnOutputs(candidates);
    }
    
    std::unordered_set<Output> changeByPeelingChain(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::PeelingChain>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByPeelingChain(const Transaction &tx);
    
    // Detects a change output by checking for output values that are multiples of 10^digits.
    std::unordered_set<Output> changeByPowerOfTenValue(const Transaction &tx, int digits) {
        return ChangeHeuristicImpl<ChangeType::PowerOfTen>{digits}(tx);
    }
    ranges::optional<Output> uniqueChangeByPowerOfTenValue(const Transaction &tx, int digits) {
        return internal::singleOrNullptr(changeByPowerOfTenValue(tx, digits));
    }
    
    std::unordered_set<Output> changeByOptimalChange(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::OptimalChange>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByOptimalChange(const Transaction &tx) {
        return internal::singleOrNullptr(changeByOptimalChange(tx));
    }
    
    std::unordered_set<Output> changeByAddressType(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::AddressType>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByAddressType(const Transaction &tx) {
        return internal::singleOrNullptr(changeByAddressType(tx));
    }
    
    std::unordered_set<Output> changeByLocktime(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::Locktime>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByLocktime(const Transaction &tx) {
        return internal::singleOrNullptr(changeByLocktime(tx));
    }
    
    std::unordered_set<Output> changeByAddressReuse(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::AddressReuse>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByAddressReuse(const Transaction &tx) {
        return internal::singleOrNullptr(changeByAddressReuse(tx));
    }
    
    std::unordered_set<Output> changeByClientChangeAddressBehavior(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>{}(tx);
    }
    ranges::optional<Output> uniqueChangeByClientChangeAddressBehavior(const Transaction &tx) {
        return internal::singleOrNullptr(changeByClientChangeAddressBehavior(tx));
    }
    
    std::unordered_set<Output> changeByLegacyHeuristic(const Transaction &tx) {
        return ChangeHeuristicImpl<ChangeType::Legacy>{}(tx);
    }

} // namespace heuristics
} // namespace blocksci

