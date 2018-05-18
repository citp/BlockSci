//
//  parallel.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/11/17.
//

#ifndef parallel_hpp
#define parallel_hpp

#include <blocksci/chain/chain_fwd.hpp>

#include <vector>
#include <future>
#include <thread>
#include <functional>
#include <stdio.h>

namespace blocksci {
    
    template <typename It, typename MapType, typename ResultType>
    ResultType mapReduceTransactionsImp(It begin, It end, const std::function<MapType(const std::vector<Block> &)> &mapFunc, const std::function<ResultType&(ResultType &, MapType &)> &reduceFunc, ResultType identity) {
        auto segmentCount = std::distance(begin, end);
        if(segmentCount == 1) {
            ResultType res = identity;
            auto ret = mapFunc(*begin);
            res = reduceFunc(res, ret);
            return res;
        } else {
            auto mid = begin;
            std::advance(mid, segmentCount / 2);
            auto handle = std::async(std::launch::async, mapReduceTransactionsImp<It, MapType, ResultType>, begin, mid, mapFunc, reduceFunc, identity);
            ResultType res = identity;
            auto ret1 = mapReduceBlocksImp(mid, end, mapFunc, reduceFunc, identity);
            res = reduceFunc(res, ret1);
            auto ret2 = handle.get();
            res = reduceFunc(res, ret2);
            return res;
        }
    }
    
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
}

#endif /* parallel_hpp */
