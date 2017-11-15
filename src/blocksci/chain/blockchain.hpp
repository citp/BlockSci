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
#include "transaction_range.hpp"

#include <blocksci/data_access.hpp>

#include <range/v3/view_facade.hpp>

#include <future>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    template <typename It, typename MapType, typename ResultType>
    ResultType mapReduceBlocksImp(It begin, It end, const std::function<MapType(const std::vector<Block> &)> &mapFunc, const std::function<ResultType&(ResultType &, MapType &)> &reduceFunc, ResultType identity) {
        auto segmentCount = std::distance(begin, end);
        if(segmentCount == 1) {
            ResultType res = identity;
            auto ret = mapFunc(*begin);
            res = reduceFunc(res, ret);
            return res;
        } else {
            auto mid = begin;
            std::advance(mid, segmentCount / 2);
            auto handle = std::async(std::launch::async, mapReduceBlocksImp<It, MapType, ResultType>, begin, mid, mapFunc, reduceFunc, identity);
            ResultType res = identity;
            auto ret1 = mapReduceBlocksImp(mid, end, mapFunc, reduceFunc, identity);
            res = reduceFunc(res, ret1);
            auto ret2 = handle.get();
            res = reduceFunc(res, ret2);
            return res;
        }
    }
    
    class Blockchain;
    
    std::vector<std::vector<Block>> segmentChain(const Blockchain &chain, int startBlock, int endBlock, unsigned int segmentCount);
    uint32_t txCount(const Blockchain &chain);
    
    class Blockchain : public ranges::view_facade<Blockchain> {
        friend ranges::range_access;
        
        struct cursor {
        private:
            const Blockchain *chain;
            uint32_t currentBlockHeight;
        public:
            cursor() = default;
            cursor(const Blockchain &chain_, uint32_t height) : chain(&chain_), currentBlockHeight(height) {}
            Block read() const;
            bool equal(ranges::default_sentinel) const;
            bool equal(const cursor &other) const;
            void next();
            void prev();
            int distance_to(cursor const &that) const;
            void advance(int amount);
        };
        
        cursor begin_cursor() const {
            return cursor(*this, 0);
        }
        
        cursor end_cursor() const {
            return cursor(*this, lastBlockHeight);
        }
        
        uint32_t lastBlockHeight;

    public:
        Blockchain() = default;
        Blockchain(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        Blockchain(const std::string &dataDirectory);
        
        const DataAccess *access;
        
        uint32_t firstTxIndex() const;
        uint32_t endTxIndex() const;
        
        uint32_t size() const {
            return lastBlockHeight;
        }
        
        TransactionRange iterateTransactions(int startBlock, int endBlock) const {
            auto startB = this->operator[](startBlock);
            auto endB = this->operator[](endBlock - 1);
            return TransactionRange(*access->chain, startB.firstTxIndex(), endB.endTxIndex());
        }

        RawTransactionRange iterateRawTransactions(int startBlock, int endBlock) const {
            auto startB = this->operator[](startBlock);
            auto endB = this->operator[](endBlock - 1);
            return RawTransactionRange(*access->chain, startB.firstTxIndex(), endB.endTxIndex());
        }
        
        template <typename MapType, typename ResultType = MapType>
        auto mapReduce(int start, int stop, const std::function<MapType(const std::vector<Block> &)> &mapFunc, const std::function<ResultType&(ResultType &, MapType &)> &reduceFunc, ResultType identity) const -> decltype(mapFunc(std::declval<std::vector<Block> &>())) {
            auto segments = segmentChain(*this, start, stop, std::thread::hardware_concurrency());
            return mapReduceBlocksImp<decltype(segments)::iterator, MapType, ResultType>(segments.begin(), segments.end(), mapFunc, reduceFunc, identity);
        }
        
        template <typename MapType, typename ResultType = MapType>
        ResultType mapReduce(int start, int stop, const std::function<MapType(const Block &)> &mapFunc, const std::function<ResultType&(ResultType &, MapType &)> &reduceFunc, ResultType identity) const {
            auto mapF = [&](const std::vector<Block> &segment) {
                ResultType res = identity;
                for (auto &block : segment) {
                    auto mapped = mapFunc(block);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduce<MapType, ResultType>(start, stop, mapF, reduceFunc, identity);
        }
        
        template <typename MapType, typename ResultType = MapType>
        ResultType mapReduce(int start, int stop, const std::function<MapType(const Transaction &)> &mapFunc, const std::function<ResultType&(ResultType &, MapType &)> &reduceFunc, ResultType identity) const {
            auto mapF = [&](const Block &block) {
                ResultType res = identity;
                for (auto tx : block) {
                    auto mapped = mapFunc(tx);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduce<MapType, ResultType>(start, stop, mapF, reduceFunc, identity);
        }
        
        template <typename MapType>
        std::vector<MapType> map(int start, int stop, const std::function<MapType(const Block &)> &mapFunc) const {
            auto mapF = [&](const std::vector<Block> &segment) {
                std::vector<MapType> vec;
                vec.reserve(segment.size());
                for (auto &block : segment) {
                    vec.push_back(mapFunc(block));
                }
                return vec;
            };
            
            auto reduceFunc = [](std::vector<MapType> &vec1, std::vector<MapType> &vec2) -> std::vector<MapType> & {
                vec1.reserve(vec1.size() + vec2.size());
                vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
                return vec1;
            };
            
            std::vector<MapType> vec;
            return mapReduce<std::vector<MapType>, std::vector<MapType>>(start, stop, mapF, reduceFunc, vec);
        }
    };
    
    // filter - Blocks and Txes
    std::vector<Block> filter(const Blockchain &chain, int startBlock, int endBlock, std::function<bool(const Block &block)> testFunc);
    std::vector<Transaction> filter(const Blockchain &chain, int startBlock, int endBlock, std::function<bool(const Transaction &tx)> testFunc);
    
    std::vector<Transaction> getCoinjoinTransactions(const Blockchain &chain, int startBlock, int endBlock);
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(const Blockchain &chain, uint64_t minBaseFee, double percentageFee, size_t maxDepth);
    std::vector<Transaction> getTransactionIncludingOutput(const Blockchain &chain, int startBlock, int endBlock, AddressType::Enum type);
    
    std::vector<Transaction> getDeanonTxes(const Blockchain &chain, int startBlock, int endBlock);
    std::vector<Transaction> getChangeOverTxes(const Blockchain &chain, int startBlock, int endBlock);
    std::vector<Transaction> getKeysetChangeTxes(const Blockchain &chain, int startBlock, int endBlock);
}


#endif /* blockchain_hpp */
