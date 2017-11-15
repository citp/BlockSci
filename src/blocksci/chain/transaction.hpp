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
#include "inout.hpp"

#include <blocksci/util.hpp>

#include <range/v3/range_fwd.hpp>
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
        
        
        Output &getOutput(uint16_t outputNum);
        Input &getInput(uint16_t inputNum);
        
        const Output &getOutput(uint16_t outputNum) const;
        const Input &getInput(uint16_t inputNum) const;
        
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
        
        using input_range = ranges::v3::iterator_range<const Input *>;
        using output_range = ranges::v3::iterator_range<const Output *>;
        
        Transaction(const ChainAccess &access, const RawTransaction *data, uint32_t txNum, uint32_t blockHeight);
        Transaction(const ChainAccess &access, uint32_t index);
        Transaction(const ChainAccess &access, uint32_t index, uint32_t height);
        
        static ranges::optional<Transaction> txWithHash(uint256 hash, const HashIndex &index, const ChainAccess &access);
        static ranges::optional<Transaction> txWithHash(std::string hash, const HashIndex &index, const ChainAccess &access);
        
        uint256 getHash(const ChainAccess &access) const;
        std::string getString() const;
        
        std::vector<OutputPointer> getOutputPointers(const InputPointer &pointer, const ChainAccess &access) const;
        std::vector<InputPointer> getInputPointers(const OutputPointer &pointer, const ChainAccess &access) const;
        
        bool operator==(const Transaction& other) const {
            return txNum == other.txNum;
        }
        
        bool operator<(const Transaction &other) const {
            return txNum < other.txNum;
        }
        
        Block block(const ChainAccess &access) const;
        
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
        
        input_range inputs() const;
        output_range outputs() const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction(uint32_t index);
        Transaction(uint32_t index, uint32_t height);
        static ranges::optional<Transaction> txWithHash(uint256 hash);
        static ranges::optional<Transaction> txWithHash(std::string hash);
        
        uint256 getHash() const;
        
        const Block &block() const;
        #endif
    };
    
    bool hasFeeGreaterThan(const Transaction &tx, uint64_t fee);
    
    bool isCoinbase(const Transaction &tx);
    bool isCoinjoin(const Transaction &tx);
    bool isDeanonTx(const Transaction &tx);
    bool isChangeOverTx(const Transaction &tx, const ScriptAccess &scripts);
    bool containsKeysetChange(const Transaction &tx, const blocksci::ScriptAccess &access);
    CoinJoinResult isPossibleCoinjoin(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    CoinJoinResult isCoinjoinExtra(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    const Output *getOpReturn(const Transaction &tx);
    
    uint64_t totalOut(const Transaction &tx);
    uint64_t totalIn(const Transaction &tx);
    uint64_t fee(const Transaction &tx);
    double feePerByte(const Transaction &tx);
    
    const Output * getChangeOutput(const Transaction &tx, const ScriptAccess &scripts);
    
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    bool containsKeysetChange(const Transaction &tx);
    
    const Output *getChangeOutput(const Transaction &tx);
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
