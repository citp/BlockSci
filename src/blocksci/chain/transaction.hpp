//
//  transaction.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_hpp
#define transaction_hpp

#include <boost/range/adaptor/transformed.hpp>

#include <vector>
#include <stdio.h>

enum class CoinJoinResult {
    True, False, Timeout
};

namespace blocksci {
    struct AddressPointer;
    class uint256;
    class ScriptAccess;
    class ChainAccess;
    class ScriptFirstSeenAccess;
    struct TransactionSummary;
    struct Input;
    struct Output;
    struct InputExpander;
    
    struct RawTransaction {
        uint32_t sizeBytes;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        
        RawTransaction(uint32_t sizeBytes, uint32_t locktime, uint16_t inputCount, uint16_t outputCount);
        
        RawTransaction(const RawTransaction &) = delete;
        RawTransaction(RawTransaction &&) = delete;
        RawTransaction &operator=(const RawTransaction &) = delete;
        RawTransaction &operator=(RawTransaction &&) = delete;
    };
    
    
    struct Transaction {
    private:
        const RawTransaction *data;
        friend TransactionSummary;
    public:
        uint32_t txNum;
        uint32_t blockHeight;
        
        using input_range = boost::iterator_range<const Input *>;
        using output_range = boost::iterator_range<const Output *>;
        
        Transaction(const RawTransaction *data, uint32_t txNum, uint32_t blockHeight);
        
        static Transaction txWithIndex(const ChainAccess &access, uint32_t index);
        static Transaction txWithIndex(const ChainAccess &access,uint32_t index, uint32_t height);
        static boost::optional<Transaction> txWithHash(const ChainAccess &access,uint256 hash);
        static boost::optional<Transaction> txWithHash(const ChainAccess &access, std::string hash);
        
        uint256 getHash(const ChainAccess &access) const;
        std::string getString() const;
        
        boost::optional<uint16_t> getOutputNum(const Input &input) const;
        
        bool operator==(const Transaction& other) const {
            return txNum == other.txNum;
        }
        
        bool operator<(const Transaction &other) const {
            return txNum < other.txNum;
        }
        
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
        
        static const Transaction &create(const ChainAccess &access, uint32_t index);
        
        input_range inputs() const;
        output_range outputs() const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        
        static const Transaction &create(uint32_t index);
        
        static Transaction txWithIndex(uint32_t index);
        static Transaction txWithIndex(uint32_t index, uint32_t height);
        static boost::optional<Transaction> txWithHash(uint256 hash);
        static boost::optional<Transaction> txWithHash(std::string hash);
        
        uint256 getHash() const;
        #endif
    };
    
    std::vector<Transaction> getTransactionsFromHashes(const ChainAccess &access, const std::vector<std::string> &txHashes);
    
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
    
    const Output * getChangeOutput(const ScriptFirstSeenAccess &scripts, const Transaction &tx);
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddresses(const ChainAccess &access, const Transaction &tx, int maxDepth, const Input &inputToReplace);
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddressesList(const ChainAccess &access, const Transaction &tx, int maxDepth, const Input &inputToReplace, const std::vector<Transaction> &cjTxes);
    
    std::string getOpReturnData(const Transaction &tx, const ScriptAccess &access);
    
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    std::vector<Transaction> getTransactionsFromHashes(const std::vector<std::string> &txHashes);
    std::string getOpReturnData(const Transaction &tx);
    bool containsKeysetChange(const Transaction &tx);
    
    const Output *getChangeOutput(const Transaction &tx);
    bool isChangeOverTx(const Transaction &tx);
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddresses(const Transaction &tx, int maxDepth, const Input &inputToReplace);
    
    std::vector<std::pair<AddressPointer, int>> getSourceAddressesList(const Transaction &tx, int maxDepth, const Input &inputToReplace, const std::vector<Transaction> &cjTxes);
    
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
