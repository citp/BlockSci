//
//  algorithms.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef chain_algorithms_hpp
#define chain_algorithms_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/numeric/accumulate.hpp>

namespace blocksci {
    
    template <typename B>
    constexpr bool isInputRange = std::is_same<ranges::range_value_type_t<B>, Input>::value;
    
    template <typename B>
    constexpr bool isOutputRange = std::is_same<ranges::range_value_type_t<B>, Output>::value;
    
    template <typename B>
    constexpr bool isTxRange = std::is_same<ranges::range_value_type_t<B>, Transaction>::value;
    
    template <typename B>
    constexpr bool isBlockRange = std::is_same<ranges::range_value_type_t<B>, Block>::value;
    
    template <typename B>
    constexpr bool isTx = std::is_same<B, Transaction>::value;
    
    template <typename B>
    constexpr bool isBlockchain = std::is_same<B, Blockchain>::value;
    
    template<typename T>
    struct fail_helper : std::false_type
    { };
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isTxRange<B>, int> = 0>
    inline auto txes(B && b) {
        return std::forward<B>(b);
    }
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isBlockRange<B>, int> = 0>
    inline auto txes(B && b) {
        return std::forward<B>(b) | ranges::view::join;
    }
    
    inline auto inputs(const Transaction &tx) {
        return tx.inputs();
    }
    
    inline auto inputs(Transaction &tx) {
        return tx.inputs();
    }
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isInputRange<B>, int> = 0>
    inline auto inputs(B && b) {
        return std::forward<B>(b);
    }
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isTxRange<B> || isBlockRange<B>, int> = 0>
    inline auto inputs(B && b) {
        return txes(std::forward<B>(b)) | ranges::view::transform([](const Transaction &tx) { return tx.inputs(); }) | ranges::view::join;
    }
    
    inline auto outputs(const Transaction &tx) {
        return tx.outputs();
    }
    
    inline auto outputs(Transaction &tx) {
        return tx.outputs();
    }
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isOutputRange<B>, int> = 0>
    inline auto outputs(B && b) {
        return std::forward<B>(b);
    }
    
    template <typename B, CONCEPT_REQUIRES_(ranges::Range<B>()), std::enable_if_t<isTxRange<B> || isBlockRange<B>, int> = 0>
    inline auto outputs(B && b) {
        return txes(std::forward<B>(b)) | ranges::view::transform([](const Transaction &tx) { return tx.outputs(); }) | ranges::view::join;
    }
    
    template <typename T>
    inline auto outputsUnspent(T && t) {
        return outputs(std::forward<T>(t)) | ranges::view::remove_if([](const Output &output) { return output.isSpent(); });
    }
    
    template <typename T>
    inline auto outputsSpentBeforeHeight(T && t, blocksci::BlockHeight blockHeight) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->blockHeight < blockHeight; });
    }
    
    template <typename T>
    inline auto outputsSpentAfterHeight(T && t, blocksci::BlockHeight blockHeight) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->blockHeight >= blockHeight; });
    }
    
    template <typename T>
    inline auto inputsCreatedAfterHeight(T && t, blocksci::BlockHeight blockHeight) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) { return input.getSpentTx().blockHeight >= blockHeight; });
    }
    
    template <typename T>
    inline auto inputsCreatedBeforeHeight(T && t, blocksci::BlockHeight blockHeight) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) { return input.getSpentTx().blockHeight < blockHeight; });
    }

    template <typename T>
    inline auto outputsSpentWithinRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { 
            return output.isSpent() && output.getSpendingTx()->blockHeight - output.blockHeight < difference; 
        });
    }
    
    template <typename T>
    inline auto outputsSpentOutsideRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { 
            return output.isSpent() && output.getSpendingTx()->blockHeight - output.blockHeight >= difference; 
        });
    }
    
    template <typename T>
    inline auto inputsCreatedWithinRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().blockHeight < difference; 
        });
    }
    
    template <typename T>
    inline auto inputsCreatedOutsideRelativeHeight(T && t, blocksci::BlockHeight difference) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().blockHeight >= difference;
        });
    }
    
    template <typename T>
    inline auto outputsOfType(T && t, AddressType::Enum type) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { return output.getType() == type; });
    }
    
    template <typename T>
    inline auto outputsOfType(T && t, EquivAddressType::Enum type) {
        return outputs(std::forward<T>(t)) | ranges::view::filter([=](const Output &output) { return equivType(output.getType()) == type; });
    }
    
    template <typename T>
    inline auto inputsOfType(T && t, AddressType::Enum type) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) { return input.getType() == type; });
    }
    
    template <typename T>
    inline auto inputsOfType(T && t, EquivAddressType::Enum type) {
        return inputs(std::forward<T>(t)) | ranges::view::filter([=](const Input &input) { return equivType(input.getType()) == type; });
    }
    
    template <typename T>
    inline uint64_t inputCount(T && t) {
        auto values = txes(std::forward<T>(t)) | ranges::view::transform([](const Transaction &tx) { return tx.inputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t outputCount(T && t) {
        auto values = txes(std::forward<T>(t)) | ranges::view::transform([](const Transaction &tx) { return tx.outputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalInputValue(T && t) {
        auto values = inputs(std::forward<T>(t)) | ranges::view::transform([](const Input &a) { return a.getValue(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalOutputValue(T && t) {
        auto values = outputs(std::forward<T>(t)) | ranges::view::transform([](const Output &a) { return a.getValue(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    inline uint64_t fee(const Transaction &tx) {
        if (tx.isCoinbase()) {
            return 0;
        } else {
            uint64_t total = 0;
            for (auto &input : tx.rawInputs()) {
                total += input.getValue();
            }
            for (auto &output : tx.rawOutputs()) {
                total -= output.getValue();
            }
            return total;
        }
    }

    template <typename T>
    inline auto feeLessThan(T &t, int64_t value) {
        return txes(t) | ranges::view::filter([=](const Transaction &tx) {
            return fee(tx) < value;
        });
    }

    template <typename T>
    inline auto feeGreaterThan(T &t, int64_t value) {
        return txes(t) | ranges::view::filter([=](const Transaction &tx) {
            return fee(tx) > value;
        });
    }
    
    template <typename T>
    inline auto fees(T &t) {
        return ranges::view::transform(txes(t), [](Transaction && tx) {
            return fee(tx);
        });
    }
    
    inline double feePerByte(const Transaction &tx) {
        return static_cast<double>(fee(tx)) / static_cast<double>(tx.sizeBytes());
    }
    
    template <typename T>
    inline auto feesPerByte(T &t) {
        return ranges::view::transform(txes(t), feePerByte);
    }
    
    template <typename T>
    inline uint64_t totalFee(T &t) {
        return ranges::accumulate(fees(t), uint64_t{0});
    }
}

#endif /* chain_algorithms_hpp */
