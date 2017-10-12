//
//  performance.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "performance.hpp"

using namespace blocksci;

std::vector<uint64_t> unspentSums1(Blockchain &chain, uint32_t start, uint32_t stop) {
    std::vector<uint64_t> sums;
    sums.reserve(stop - start);
    for (uint32_t height = start; height < stop; height++) {
        auto &block = chain[height];
        uint64_t total = 0;
        for (const auto tx : block) {
            for (const auto &output : tx.outputs()) {
                if (!output.isSpent()) {
                    total += output.getValue();
                }
            }
        }
        sums.push_back(total);
    }
    return sums;
}

std::vector<uint64_t> unspentSums2(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto func = [](const Block &block) {
        uint64_t total = 0;
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                if (!output.isSpent()) {
                    total += output.getValue();
                }
            }
        }
        return total;
    };
    
    return chain.map<uint64_t>(start, stop, func);
}

uint32_t maxSizeTx1(Blockchain &chain, uint32_t start, uint32_t stop) {
    uint32_t max = 0;
    for (uint32_t height = start; height < stop; height++) {
        auto &block = chain[height];
        for (const auto tx : block) {
            max = std::max(max, tx.sizeBytes());
        }
    }
    return max;
}

uint32_t maxSizeTx2(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) { return tx.sizeBytes(); };
    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<uint32_t, uint32_t>(start, stop, extract, combine, uint32_t(0));
}

std::unordered_map<uint64_t, uint64_t> getOutputDistribution1(Blockchain &chain, uint32_t start, uint32_t stop) {
    
    std::unordered_map<uint64_t, uint64_t> distribution;
    for (uint32_t height = start; height < stop; height++) {
        auto &block = chain[height];
        for (auto tx : block) {
            for(auto &output : tx.outputs()) {
                auto value = output.getValue();
                value &= ~0xFF;
                auto it = distribution.insert(std::make_pair(value, 0));
                it.first->second++;
            }
        }
        
    }
    return distribution;
}

std::unordered_map<uint64_t, uint64_t> getOutputDistribution2(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto mapFunc = [](const std::vector<Block> &segment) {
        std::unordered_map<uint64_t, uint64_t> distribution;
        for (auto &block : segment) {
            for (auto tx : block) {
                for(auto &output : tx.outputs()) {
                    auto value = output.getValue();
                    value &= ~0xFF;
                    auto it = distribution.insert(std::make_pair(value, 0));
                    it.first->second++;
                }
            }
        }
        return distribution;
    };
    
    auto reduceFunc = [] (std::unordered_map<uint64_t, uint64_t> &map1, std::unordered_map<uint64_t, uint64_t> &map2) -> std::unordered_map<uint64_t, uint64_t> & {
        for (auto &pair : map2) {
            auto res = map1.insert(pair);
            if (!res.second) {
                res.first->second += pair.second;
            }
        }
        return map1;
    };
    
    std::unordered_map<uint64_t, uint64_t>  map;
    return chain.mapReduce<std::unordered_map<uint64_t, uint64_t>, std::unordered_map<uint64_t, uint64_t>>(start, stop, mapFunc, reduceFunc, map);
}

uint64_t maxValOutput1(Blockchain &chain, uint32_t start, uint32_t stop) {
    uint64_t maxValue = 0;
    for (uint32_t height = start; height < stop; height++) {
        auto &block = chain[height];
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                maxValue = std::max(maxValue, output.getValue());
            }
        }
    }
    return maxValue;
}

uint64_t maxValOutput2(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) {
        uint64_t maxValue = 0;
        for(auto &output : tx.outputs()) {
            maxValue = std::max(output.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](uint64_t &a, uint64_t &b) -> uint64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<uint64_t, uint64_t>(start, stop, extract, combine, uint64_t(0));
}
