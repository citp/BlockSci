//
//  transaction.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_hpp
#define transaction_hpp

#include "chain_fwd.hpp"
#include "output.hpp"
#include "input.hpp"
#include "inout.hpp"

#include <blocksci/util.hpp>

#include <range/v3/iterator_range.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/utility/optional.hpp>

#include <vector>

enum class CoinJoinResult {
    True, False, Timeout
};

namespace blocksci {
    struct Address;
    class uint256;
    class ScriptAccess;
    class HashIndex;
    
    struct RawTransaction {
        uint32_t sizeBytes;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        InPlaceArray<Inout> inOuts;
        
        RawTransaction(uint32_t sizeBytes, uint32_t locktime, uint16_t inputCount, uint16_t outputCount);
        
        RawTransaction(const RawTransaction &) = delete;
        RawTransaction(RawTransaction &&) = delete;
        RawTransaction &operator=(const RawTransaction &) = delete;
        RawTransaction &operator=(RawTransaction &&) = delete;
        
        
        Inout &getOutput(uint16_t outputNum);
        Inout &getInput(uint16_t inputNum);
        
        const Inout &getOutput(uint16_t outputNum) const;
        const Inout &getInput(uint16_t inputNum) const;
        
        size_t realSize() const {
            return sizeof(RawTransaction) + inOuts.extraSize();
        }
    };
    
    struct Transaction {
    private:
        const ChainAccess *access;
        const RawTransaction *data;
        friend TransactionSummary;
    public:
        uint32_t txNum;
        uint32_t blockHeight;
        
        Transaction(const RawTransaction *data, uint32_t txNum, uint32_t blockHeight, const ChainAccess &access);
        Transaction(uint32_t index, const ChainAccess &access);
        Transaction(uint32_t index, uint32_t height, const ChainAccess &access);
        
        static ranges::optional<Transaction> txWithHash(uint256 hash, const HashIndex &index, const ChainAccess &access);
        static ranges::optional<Transaction> txWithHash(std::string hash, const HashIndex &index, const ChainAccess &access);
        
        uint256 getHash() const;
        std::string getString() const;
        
        std::vector<OutputPointer> getOutputPointers(const InputPointer &pointer) const;
        std::vector<InputPointer> getInputPointers(const OutputPointer &pointer) const;
        
        uint32_t sizeBytes() const {
            return data->sizeBytes;
        }
        
        uint32_t locktime() const {
            return data->locktime;
        }
        
        uint16_t inputCount() const {
            return data->inputCount;
        }
        
        uint16_t outputCount() const {
            return data->outputCount;
        }
        
        ranges::iterator_range<const Inout *> rawOutputs() const {
            auto &firstOut = data->getOutput(0);
            return ranges::make_iterator_range(&firstOut, &firstOut + outputCount());
        }
        
        ranges::iterator_range<const Inout *> rawInputs() const {
            auto &firstIn = data->getInput(0);
            return ranges::make_iterator_range(&firstIn, &firstIn + inputCount());
        }
        
        auto outputs() const {
            return rawOutputs() | ranges::view::transform([&](const Inout &inout) { return Output(inout, *access); });
        }
        
        auto inputs() const {
            return rawInputs() | ranges::view::transform([&](const Inout &inout) { return Input(inout, *access); });
        }
        
        Block block() const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction(uint32_t index);
        Transaction(uint32_t index, uint32_t height);
        static ranges::optional<Transaction> txWithHash(uint256 hash);
        static ranges::optional<Transaction> txWithHash(std::string hash);
        #endif
    };
    
    inline bool operator==(const Transaction& a, const Transaction& b) {
        return a.txNum == b.txNum;
    }
    
    inline bool operator<(const Transaction& a, const Transaction& b) {
        return a.txNum < b.txNum;
    }
    
    using input_range = decltype(std::declval<Transaction>().inputs());
    using output_range = decltype(std::declval<Transaction>().outputs());
    
    bool hasFeeGreaterThan(const Transaction &tx, uint64_t fee);
    
    inline bool isCoinbase(const Transaction &tx) {
        return tx.inputCount() == 0;
    }
    
    bool isCoinjoin(const Transaction &tx);
    bool isDeanonTx(const Transaction &tx);
    bool isChangeOverTx(const Transaction &tx, const ScriptAccess &scripts);
    bool containsKeysetChange(const Transaction &tx, const blocksci::ScriptAccess &access);
    CoinJoinResult isPossibleCoinjoin(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    CoinJoinResult isCoinjoinExtra(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    ranges::optional<Output> getOpReturn(const Transaction &tx);
    
    inline uint64_t totalOut(const Transaction &tx) {
        auto values = tx.outputs() | ranges::view::transform([](const Output &output) { return output.getValue(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    inline uint64_t totalIn(const Transaction &tx) {
        auto values = tx.inputs() | ranges::view::transform([](const Input &input) { return input.getValue(); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    inline uint64_t fee(const Transaction &tx) {
        if (isCoinbase(tx)) {
            return 0;
        } else {
            return totalIn(tx) - totalOut(tx);
        }
    }
    
    double feePerByte(const Transaction &tx);
    
    ranges::optional<Output> getChangeOutput(const Transaction &tx, const ScriptAccess &scripts);
    
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    bool containsKeysetChange(const Transaction &tx);
    
    ranges::optional<Output> getChangeOutput(const Transaction &tx);
    bool isChangeOverTx(const Transaction &tx);
    
    #endif
}


namespace std {
    template <>
    struct hash<blocksci::Transaction> {
        typedef blocksci::Transaction argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return static_cast<size_t>(b.txNum);
        }
    };
}


#endif /* transaction_hpp */
