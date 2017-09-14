//
//  block.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_hpp
#define block_hpp

#include <blocksci/bitcoin_uint256.hpp>
#include <blocksci/address/address_types.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <unordered_map>
#include <chrono>
#include <stdio.h>

namespace blocksci {
    
    class TransactionIterator;
    struct Transaction;
    struct TransactionSummary;
    struct Output;
    class ChainAccess;
    class ScriptAccess;
    
    struct Block {
        
        uint32_t firstTxIndex;
        uint32_t numTxes;
        uint32_t height;
        uint256 hash;
        int32_t version;
        uint32_t timestamp;
        uint32_t bits;
        uint32_t nonce;
        uint64_t coinbaseOffset;
        
        using value_type = Transaction;
        using const_iterator = TransactionIterator;
        using iterator = TransactionIterator;
        using size_type = size_t;
        using TransactionRange = boost::iterator_range<TransactionIterator>;
        
        Block(uint32_t firstTxIndex, uint32_t numTxes, uint32_t height, uint256 hash, int32_t version, uint32_t timestamp, uint32_t bits, uint32_t nonce, uint64_t coinbaseOffset);
        
        bool operator==(const Block& other) const;
        
        const std::string getHeaderHash() const {
            return hash.GetHex();
        }
        
        std::chrono::system_clock::time_point getTime() const;
        
        size_t size() const { return numTxes; }
        
        std::string getString() const;
        
        static const Block &create(const ChainAccess &access, uint32_t height);
        
        TransactionRange txes(const ChainAccess &access) const;
        
        const_iterator begin(const ChainAccess &access) const;
        const_iterator end(const ChainAccess &access) const;
        
        std::vector<unsigned char> getCoinbase(const ChainAccess &access) const;
        
        const std::string coinbaseParam(const ChainAccess &access) const {
            auto coinbase = getCoinbase(access);
            return std::string(coinbase.begin(), coinbase.end());
        }
        
        Transaction coinbaseTx(const ChainAccess &access) const;
        bool isSegwit(const ChainAccess &access) const;
        
        value_type getTx(const ChainAccess &access, uint32_t txNum) const;
        
        // DataAccess required
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        
        static const Block &create(uint32_t height);

        TransactionRange txes() const;
        
        const_iterator begin() const;
        const_iterator end() const;
        
        std::vector<unsigned char> getCoinbase() const;
        
        const std::string coinbaseParam() const {
            auto coinbase = getCoinbase();
            return std::string(coinbase.begin(), coinbase.end());
        }

        Transaction coinbaseTx() const;

        value_type operator[](uint32_t txNum) const;
        
        #endif
    };
    
    size_t sizeBytes(const Block &block, const ChainAccess &access);
    bool isSegwit(const Block &block, const ChainAccess &access, const ScriptAccess &scripts);
    std::vector<uint64_t> fees(const Block &block, const ChainAccess &access);
    std::vector<double> feesPerByte(const Block &block, const ChainAccess &access);
    TransactionSummary transactionStatistics(const Block &block, const ChainAccess &access);
    std::vector<const Output *> getUnspentOutputs(const Block &block, const ChainAccess &access);
    std::vector<const Output *> getOutputsSpentByHeight(const Block &block, uint32_t height, const ChainAccess &access);
    uint64_t totalOut(const Block &block, const ChainAccess &access);
    uint64_t totalIn(const Block &block, const ChainAccess &access);
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height, const ChainAccess &access);
    uint64_t getTotalSpentOfAge(const Block &block, const ChainAccess &access, uint32_t age);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, const ChainAccess &access, uint32_t maxAge);
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block, const ChainAccess &access);
    
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    size_t sizeBytes(const Block &block);
    bool isSegwit(const Block &block);
    uint64_t totalOut(const Block &block);
    uint64_t totalIn(const Block &block);
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height);
    std::vector<uint64_t> fees(const Block &block);
    std::vector<double> feesPerByte(const Block &block);
    std::vector<const Output *> getUnspentOutputs(const Block &block);
    std::vector<const Output *> getOutputsSpentByHeight(const Block &block, uint32_t height);
    uint64_t getTotalSpentOfAge(const Block &block, uint32_t age);
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, uint32_t maxAge);
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block);
    #endif
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &output);

#endif /* block_hpp */
