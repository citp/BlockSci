//
//  blockchain.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blockchain_hpp
#define blockchain_hpp

#include "block.hpp"
#include "transaction.hpp"
#include "transaction_iterator.hpp"
#include <blocksci/data_access.hpp>

#include <boost/type_traits/function_traits.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/iterator_range.hpp>

#include <future>
#include <stdio.h>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    namespace detail {
        template <typename T>
        using always_void = void;
        
        template <typename Expr, typename Enable = void>
        struct is_callable_impl : std::false_type {};
        
        template <typename F, typename... Args>
        struct is_callable_impl<F(Args...), always_void<typename std::result_of<F(Args...)>::type>> : std::true_type {};
    }
    
    template <typename Expr, typename... Args>
    struct is_callable : detail::is_callable_impl<Expr, Args...> {};
    
    template <typename It, typename MapFunc, typename ReduceFunc, typename ResultType>
    ResultType mapReduceBlocksImp(It begin, It end, MapFunc mapFunc, ReduceFunc reduceFunc, ResultType identity) {
        auto segmentCount = std::distance(begin, end);
        if(segmentCount == 1) {
            ResultType res = identity;
            auto ret = mapFunc(*begin);
            res = reduceFunc(res, ret);
            return res;
        } else {
            auto mid = begin;
            std::advance(mid, segmentCount / 2);
            auto handle = std::async(std::launch::async, mapReduceBlocksImp<It, MapFunc, ReduceFunc, ResultType>, begin, mid, mapFunc, reduceFunc, identity);
            ResultType res = identity;
            auto ret1 = mapReduceBlocksImp(mid, end, mapFunc, reduceFunc, identity);
            res = reduceFunc(res, ret1);
            auto ret2 = handle.get();
            res = reduceFunc(res, ret2);
            return res;
        }
    }
    
    class Blockchain;
    
    std::vector<std::vector<Block>> segmentChain(const Blockchain &chain, size_t startBlock, size_t endBlock, unsigned int segmentCount);
    uint32_t txCount(const Blockchain &chain);
    
    class Blockchain {
    public:
        using value_type = const Block;
        using const_iterator = std::vector<Block>::const_iterator;
        using iterator = std::vector<Block>::iterator;
        using size_type = std::vector<Block>::size_type;
        
        Blockchain(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        Blockchain(const std::string &dataDirectory);
        
        const DataAccess &access;
        
        Transaction txAtIndex(uint32_t index) const;
        
        std::vector<Block>::size_type size() const;
        boost::iterator_range<const Block *>::const_iterator begin() const;
        boost::iterator_range<const Block *>::const_iterator end() const;
        const Block& operator[] (const uint32_t index) const;
        
        boost::iterator_range<TransactionIterator> iterateTransactions(uint32_t startBlock, uint32_t endBlock) const {
            auto &startB = this->operator[](startBlock);
            auto &endB = this->operator[](endBlock - 1);
            auto begin = TransactionIterator(&access.chain, startB.firstTxIndex, startBlock);
            auto end = TransactionIterator(&access.chain, endB.firstTxIndex + endB.numTxes, endBlock);
            return boost::make_iterator_range(begin, end);
        }
        
        TransactionIterator beginTransactions(uint32_t blockNum);
        TransactionIterator endTransactions(uint32_t blockNum);
        
        template <typename MapFunc, typename ReduceFunc, typename ResultType>
        auto mapReduceBlockRanges(size_t start, size_t stop, MapFunc mapFunc, ReduceFunc reduceFunc, ResultType identity) const -> decltype(mapFunc(std::declval<std::vector<Block> &>())) {
            auto segments = segmentChain(*this, start, stop, std::thread::hardware_concurrency());
            return mapReduceBlocksImp(segments.begin(), segments.end(), mapFunc, reduceFunc, identity);
        }
        
        // Use std::function to specify return type
        // all called mapReduce
        template <typename MapFunc, typename ReduceFunc, typename ResultType>
        ResultType mapReduceBlocks(size_t start, size_t stop, MapFunc mapFunc, ReduceFunc reduceFunc, ResultType identity) const {
            auto mapF = [&](std::vector<Block> &segment) {
                ResultType res = identity;
                for (auto &block : segment) {
                    auto mapped = mapFunc(block);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduceBlockRanges(start, stop, mapF, reduceFunc, identity);
        }
        
        template <typename MapFunc, typename ReduceFunc, typename ResultType>
        ResultType mapReduceTransactions(size_t start, size_t stop, MapFunc mapFunc, ReduceFunc reduceFunc, ResultType identity) {
            auto mapF = [&](const Block &block) {
                ResultType res = identity;
                for (auto tx : block.txes(access.chain)) {
                    auto mapped = mapFunc(tx);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduceBlocks(start, stop, mapF, reduceFunc, identity);
        }
        
        template <typename MapFunc,  typename ResultType = decltype(std::declval<MapFunc>()(std::declval<const Block &>()))>
        std::vector<ResultType> mapBlocks(size_t start, size_t stop, MapFunc mapFunc) const {
            auto mapF = [&](std::vector<Block> &segment) {
                std::vector<ResultType> vec;
                vec.reserve(segment.size());
                for (auto &block : segment) {
                    vec.push_back(mapFunc(block));
                }
                return vec;
            };
            
            auto reduceFunc = [](std::vector<ResultType> &vec1, std::vector<ResultType> &vec2) {
                vec1.reserve(vec1.size() + vec2.size());
                vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
                return vec1;
            };
            
            std::vector<ResultType> vec;
            return mapReduceBlockRanges(start, stop, mapF, reduceFunc, vec);
        }
    };
    
    // filter - Blocks and Txes
    std::vector<Transaction> getMatchingTransactions(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, std::function<bool(const Transaction &tx)> testFunc);
    
    std::vector<Transaction> getCoinjoinTransactions(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock);
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(const Blockchain &chain, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    std::vector<Transaction> getTransactionIncludingOutput(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, AddressType::Enum type);
    
    std::vector<Transaction> getDeanonTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock);
    std::vector<Transaction> getChangeOverTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock);
    std::vector<Transaction> getKeysetChangeTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock);
}


#endif /* blockchain_hpp */
