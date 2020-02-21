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
#include <range/v3/view/filter.hpp>

#include <unordered_set>
#include <cmath>


/** Change address heuristics
 *
 * Every heuristic returns the set of outputs it cannot rule out as change.
 */
namespace blocksci { namespace heuristics {
    
    /** Remove OP_RETURN outputs from candidate set */
    bool filterOpReturn(Output o) {
        return o.getAddress().isSpendable();
    }
    
    /** In a peeling chain, the change output is the output that continues the chain
     *
     * Note: This heuristic depends on the outputs being spent to detect change.
     * If an output has not been spent, it is considered a potential change output.
     */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::PeelingChain>::operator()(const Transaction &tx) const {
        // If current tx is not a peeling chain, return an empty set
        if (!isPeelingChain(tx)) {
            return ranges::view::empty<Output>;
        }
        
        // Check which output(s) continue the peeling chain
        return tx.outputs() | ranges::view::filter([](Output o){return !o.isSpent() || isPeelingChain(*o.getSpendingTx());}) | ranges::view::filter(filterOpReturn);
    }

    /** Returns 10^{digits} */
    int64_t int_pow_ten(int digits) {
        if (digits < 16) {
            int64_t values[16] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000, 1000000000000000};
            return values[digits];
        } else {
            int64_t value = 1000000000000000;
            for (int i = 15; i < digits; ++i) {
                value *= 10;
            }
            return value;
        }
    }
    
    /** When users transfer bitcoins between their own wallets, they are likely to choose values that are powers of ten.
     *
     * On the other hand, it is extremely unlikely that you receive power of ten change due to a wallet's coin selection.
     * Default for digits is 6 (i.e. it selects outputs with a value that is a multiple of 0.01 BTC)
     */
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::PowerOfTen>::operator()(const Transaction &tx) const {
        int64_t value = int_pow_ten(digits);
        return tx.outputs() | ranges::view::filter([value](Output o){return o.getValue() % value != 0;}) | ranges::view::filter(filterOpReturn);
    }
    
    
    /** If there exists an output that is smaller than any of the inputs it is likely the change.
     *
     * If a change output was larger than the smallest input, then the coin selection algorithm
     * wouldn't need to add the input in the first place.
     */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::OptimalChange>::operator()(const Transaction &tx) const {
        auto smallestInputValue = tx.inputs()[0].getValue();
        RANGES_FOR(auto input, tx.inputs()) {
            smallestInputValue = std::min(smallestInputValue, input.getValue());
        }
        return tx.outputs() | ranges::view::filter([smallestInputValue](Output o){return o.getValue() < smallestInputValue;}) | ranges::view::filter(filterOpReturn);
    }
    
    /** If all inputs are of one address type (e.g., P2PKH or P2SH), it is likely that the change output has the same type. */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::AddressType>::operator()(const Transaction &tx) const {
        // check whether all inputs have the same type (e.g., P2SH)
        bool allInputsSameType = true;
        AddressType::Enum inputType = tx.inputs()[0].getType();
        RANGES_FOR(auto input, tx.inputs()) {
            if (input.getType() != inputType) {
                allInputsSameType = false;
                break;
            }
        }
        
        if (allInputsSameType) {
            return tx.outputs() | ranges::view::filter([inputType](Output o){return o.getType() == inputType;}) | ranges::view::filter(filterOpReturn);
        } else {
            return ranges::view::empty<Output>;
        }
    }
    
    /** Detects change based on a transaction's locktime
     *
     * Bitcoin Core sets the locktime to the current block height to prevent fee sniping.
     * If all outputs have been spent, and there is only one output that has been spent
     * in a transaction that matches this transaction's locktime behavior, it is the change.
     * This heuristic depends on the outputs being spent to detect change.
     * If an output has not been spent, it is considered a potential change output.
     */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::Locktime>::operator()(const Transaction &tx) const {
        bool locktimeGreaterZero = tx.locktime() > 0;
        return tx.outputs() | ranges::view::filter([locktimeGreaterZero](Output o){return !o.isSpent() || (o.getSpendingTx().value().locktime() > 0) == locktimeGreaterZero;}) | ranges::view::filter(filterOpReturn);
    }

    /** If input addresses appear as an output address, the client might have reused addresses for change. */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::AddressReuse>::operator()(const Transaction &tx) const {
        std::unordered_set<Address> inputAddresses;
        RANGES_FOR(auto input, tx.inputs()) {
            inputAddresses.insert(input.getAddress());
        }
        
        return tx.outputs() | ranges::view::filter([inputAddresses](Output o){return inputAddresses.find(o.getAddress()) != inputAddresses.end();}) | ranges::view::filter(filterOpReturn);
    }

    /** Most clients will generate a fresh address for the change.
     *
     * If an output is the first to send value to an address, it is potentially the change.
     */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>::operator()(const Transaction &tx) const {
        return tx.outputs() | ranges::view::filter([tx](Output o){return o.getAddress().isSpendable() && o.getAddress().getBaseScript().getFirstTxIndex() == tx.txNum;}) | ranges::view::filter(filterOpReturn);
    }
    
    /** Legacy heuristic used in previous versions of BlockSci */
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx) {
        if (isCoinjoin(tx)) {
            return ranges::nullopt;
        }
        
        auto smallestInput = std::numeric_limits<int64_t>::max();
        RANGES_FOR(auto input, tx.inputs()) {
            smallestInput = std::min(smallestInput, input.getValue());
        }
        
        uint16_t spendableCount = 0;
        ranges::optional<Output> change;
        RANGES_FOR(auto output, tx.outputs()) {
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
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::Legacy>::operator()(const Transaction &tx) const {
        auto c = uniqueChangeByLegacyHeuristic(tx);
        if (c.has_value()) {
            return ranges::view::single(c.value());
        }
        return ranges::view::empty<Output>;
    }

    /** Clients may choose a fixed fee per kb instead of using one based on the current fee market. */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::FixedFee>::operator()(const Transaction &tx) const {
        auto fee = tx.fee() * 1000 / tx.virtualSize();
        return tx.outputs() | ranges::view::filter([fee](Output o) {return !o.isSpent() || (o.getSpendingTx()->fee() * 1000 / o.getSpendingTx()->virtualSize()) == fee;}) | ranges::view::filter(filterOpReturn);
    }
    
    /** Disables change address clustering by returning an empty set. */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::None>::operator()(const Transaction &) const {
        return ranges::view::empty<Output>;
    }
    
    /** Returns all outputs that have been spent.
     *
     * This is useful in combination with change address heuristics that return unspent outputs as candidates.
     */
    template<>
    ranges::any_view<Output> ChangeHeuristicImpl<ChangeType::Spent>::operator()(const Transaction &tx) const {
        return tx.outputs() | ranges::view::filter([](Output o){return o.isSpent();});
    }
}  // namespace heuristics
}  // namespace blocksci
