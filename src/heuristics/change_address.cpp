//
//  change_address.cpp
//  blocksci
//
//  Created by Malte Möser on 10/2/17.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/range_for.hpp>

#include <unordered_set>
#include <cmath>


// Common change address heuristics
// Every heuristic returns the set of outputs it cannot rule out as change

namespace blocksci { namespace heuristics {
    
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
        // Check if any future transaction is peeling chain
        RANGES_FOR(auto output, tx.outputs()) {
            if(output.isSpent() && looksLikePeelingChain(*output.getSpendingTx())) {
                return true;
            }
        }
        return false;
    }
    
    // In a peeling chain, the change output is the output that continues the chain
    // This heuristic depends on the outputs being spent to detect change.
    // If an output has not been spent, it is considered a potential change output.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::PeelingChain>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        // If current tx is not a peeling chain, return an empty set
        if(!isPeelingChain(tx)) {
            return candidates;
        }
        
        // Check which output(s) continue the peeling chain
        RANGES_FOR(auto output, tx.outputs()) {
            if(output.isSpent()) {
                if(isPeelingChain(*output.getSpendingTx())) {
                    candidates.insert(output);
                }
            } else { // not spent, hence unknown
                candidates.insert(output);
            }
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
        RANGES_FOR(auto output, tx.outputs()) {
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
        RANGES_FOR (auto input, tx.inputs()) {
            smallestInputValue = std::min(smallestInputValue, input.getValue());
        }
        
        RANGES_FOR(Output output, tx.outputs()) {
            if(output.getValue() < smallestInputValue) {
                candidates.insert(output);
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // If all inputs are of one address type (e.g., P2PKH or P2SH),
    // it is likely that the change output has the same type.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::AddressType>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        // check whether all inputs have the same type (e.g., P2SH)
        bool allInputsSameType = true;
        AddressType::Enum inputType = tx.inputs()[0].getType();
        RANGES_FOR (auto input, tx.inputs()) {
            if(input.getType() != inputType) {
                allInputsSameType = false;
                break;
            }
        }
        if(allInputsSameType) {
            RANGES_FOR (auto output, tx.outputs()) {
                if(output.getType() == inputType) {
                    candidates.insert(output);
                }
            }
        }
        return removeOpReturnOutputs(candidates);
    }
    
    // Bitcoin Core sets the locktime to the current block height to prevent fee sniping.
    // If all outputs have been spent, and there is only one output that has been spent
    // in a transaction that matches this transaction's locktime behavior, it is the change.
    // This heuristic depends on the outputs being spent to detect change.
    // If an output has not been spent, it is considered a potential change output.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::Locktime>::operator()(const Transaction &tx) const {
        std::unordered_set<Output> candidates;
        
        // locktime of tx
        bool locktimeGreaterZero = tx.locktime() > 0;
        
        RANGES_FOR(auto output, tx.outputs()) {
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
        RANGES_FOR (auto input, tx.inputs()) {
            inputAddresses.insert(input.getAddress());
        }
        
        RANGES_FOR (auto output, tx.outputs()) {
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
        
        RANGES_FOR (auto output, tx.outputs()) {
            if (output.getAddress().isSpendable() && output.getAddress().getBaseScript().getFirstTxIndex() == tx.txNum) {
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
        RANGES_FOR (auto input, tx.inputs()) {
            smallestInput = std::min(smallestInput, input.getValue());
        }
        
        uint16_t spendableCount = 0;
        ranges::optional<Output> change;
        RANGES_FOR (auto output, tx.outputs()) {
            if (output.getAddress().isSpendable()) {
                spendableCount++;
                if (output.getValue() < smallestInput && output.getAddress().getBaseScript().getFirstTxIndex() == tx.txNum) {
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
    
    // Disables change address clustering by returning an empty set.
    template<>
    std::unordered_set<Output> ChangeHeuristicImpl<ChangeType::None>::operator()(const Transaction &) const {
        std::unordered_set<Output> candidates;
        return candidates;
    }
    
    template<>
    }
    
} // namespace heuristics
} // namespace blocksci

