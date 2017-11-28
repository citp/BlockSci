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
    constexpr bool isInputRange = std::is_same_v<ranges::range_value_type_t<B>, Input>;
    
    template <typename B>
    constexpr bool isOutputRange = std::is_same_v<ranges::range_value_type_t<B>, Output>;
    
    template <typename B>
    constexpr bool isTxRange = std::is_same_v<ranges::range_value_type_t<B>, Transaction>;
    
    template <typename B>
    constexpr bool isBlockRange = std::is_same_v<ranges::range_value_type_t<B>, Block>;
    
    template <typename B>
    constexpr bool isTx = std::is_same_v<B, Transaction>;
    
    template <typename B>
    constexpr bool isBlockchain = std::is_same_v<B, Blockchain>;
    
    template<typename T>
    struct fail_helper : std::false_type
    { };
    
    template <typename B>
    inline auto txes(B &b) {
        if constexpr (isTxRange<B>) {
            return b;
        } else if constexpr (isBlockRange<B> || isBlockchain<B>) {
            auto c = b | ranges::view::join;
            static_assert(isTxRange<decltype(c)>);
            return c;
        } else {
            static_assert(fail_helper<B>::value);
        }
    }
    
    inline auto inputs(const Transaction &tx) {
        return tx.inputs();
    }
    
    template <typename T>
    inline auto inputs(T &t) {
        if constexpr (std::is_same_v<T, Transaction>) {
            return t.inputs();
        } else if constexpr (isInputRange<T>) {
            return t;
        } else if constexpr (isTxRange<T>) {
            return t | ranges::view::transform([](const Transaction &tx) { return tx.inputs(); }) | ranges::view::join;
        } else if constexpr (isBlockRange<T>) {
            auto a = txes(t);
            static_assert(isTxRange<decltype(a)>);
            return a | ranges::view::transform([](const Transaction &tx) { return tx.inputs(); }) | ranges::view::join;
        } else {
            static_assert(fail_helper<T>::value);
        }
    }
    
    
    inline auto outputs(const Transaction &tx) {
        return tx.outputs();
    }
    
    template <typename T>
    inline auto outputs(T &t) {
        if constexpr (std::is_same_v<T, Transaction>) {
            return t.outputs();
        } else if constexpr (isOutputRange<T>) {
            return t;
        } else if constexpr (isTxRange<T>) {
            return t | ranges::view::transform([](const Transaction &tx) { return tx.outputs(); }) | ranges::view::join;
        } else if constexpr (isBlockRange<T>) {
            auto a = txes(t);
            static_assert(isTxRange<decltype(a)>);
            return a | ranges::view::transform([](const Transaction &tx) { return tx.outputs(); }) | ranges::view::join;
        } else {
            static_assert(fail_helper<T>::value);
        }
    }
    
    template <typename T>
    inline auto outputsUnspent(T &t) {
        return outputs(t) | ranges::view::remove_if([](const Output &output) { return output.isSpent(); });
    }
    
    template <typename T>
    inline auto outputsSpentBeforeHeight(T &t, uint32_t blockHeight) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->blockHeight < blockHeight; });
    }
    
    template <typename T>
    inline auto outputsSpentAfterHeight(T &t, uint32_t blockHeight) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { return output.isSpent() && output.getSpendingTx()->blockHeight >= blockHeight; });
    }
    
    template <typename T>
    inline auto inputsCreatedAfterHeight(T &t, uint32_t blockHeight) {
        return inputs(t) | ranges::view::filter([=](const Input &input) { return input.getSpentTx().blockHeight >= blockHeight; });
    }
    
    template <typename T>
    inline auto inputsCreatedBeforeHeight(T &t, uint32_t blockHeight) {
        return inputs(t) | ranges::view::filter([=](const Input &input) { return input.getSpentTx().blockHeight < blockHeight; });
    }

    template <typename T>
    inline auto outputsSpentWithinRelativeHeight(T &t, int difference) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { 
            return output.isSpent() && output.getSpendingTx()->blockHeight - output.blockHeight < difference; 
        });
    }
    
    template <typename T>
    inline auto outputsSpentOutsideRelativeHeight(T &t, int difference) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { 
            return output.isSpent() && output.getSpendingTx()->blockHeight - output.blockHeight >= difference; 
        });
    }
    
    template <typename T>
    inline auto inputsCreatedWithinRelativeHeight(T &t, int difference) {
        return inputs(t) | ranges::view::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().blockHeight < difference; 
        });
    }
    
    template <typename T>
    inline auto inputsCreatedOutsideRelativeHeight(T &t, int difference) {
        return inputs(t) | ranges::view::filter([=](const Input &input) {
            return input.blockHeight - input.getSpentTx().blockHeight >= difference;
        });
    }
    
    template <typename T>
    inline auto outputsOfAddressType(T &t, AddressType::Enum type) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { return output.getType() == type; });
    }
    
    template <typename T>
    inline auto outputsOfScriptType(T &t, ScriptType::Enum type) {
        return outputs(t) | ranges::view::filter([=](const Output &output) { return scriptType(output.getType()) == type; });
    }
    
    template <typename T>
    inline uint64_t inputCount(T &t) {
        auto values = txes(t) | ranges::view::transform([](const Transaction &tx) { return tx.inputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t outputCount(T &t) {
        auto values = txes(t) | ranges::view::transform([](const Transaction &tx) { return tx.outputCount(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalInputValue(T &t) {
        auto values = inputs(t) | ranges::view::transform([](const Input &a) { return a.getValue(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalOutputValue(T &t) {
        auto values = outputs(t) | ranges::view::transform([](const Output &a) { return a.getValue(); });
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
