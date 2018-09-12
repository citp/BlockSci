//
//  performance.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "performance.hpp"

using namespace blocksci;

std::vector<int64_t> unspentSums1(Blockchain &chain, int start, int stop) {
    std::vector<int64_t> sums;
    sums.reserve(stop - start);
    for (int height = start; height < stop; height++) {
        auto block = chain[height];
        int64_t total = 0;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                if (!output.isSpent()) {
                    total += output.getValue();
                }
            }
        }
        sums.push_back(total);
    }
    return sums;
}

std::vector<int64_t> unspentSums2(Blockchain &chain, int start, int stop) {
    auto func = [](const Block &block) {
        int64_t total = 0;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                if (!output.isSpent()) {
                    total += output.getValue();
                }
            }
        }
        return total;
    };
    
    return chain[{start, stop}].map<int64_t>(func);
}

uint32_t maxSizeTx1(Blockchain &chain, int start, int stop) {
    uint32_t max = 0;
    for (int height = start; height < stop; height++) {
        auto block = chain[height];
        RANGES_FOR(auto tx, block) {
            max = std::max(max, tx.sizeBytes());
        }
    }
    return max;
}

uint32_t maxSizeTx2(Blockchain &chain, int start, int stop) {
    auto extract = [](const Transaction &tx) { return tx.sizeBytes(); };
    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a = std::max(a,b); return a; };
    
    return chain[{start, stop}].mapReduce<uint32_t>(extract, combine);
}

std::unordered_map<int64_t, int64_t> getOutputDistribution1(Blockchain &chain, int start, int stop) {
    
    std::unordered_map<int64_t, int64_t> distribution;
    for (int height = start; height < stop; height++) {
        auto block = chain[height];
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                auto value = output.getValue();
                value &= ~0xFF;
                auto it = distribution.insert(std::make_pair(value, 0));
                it.first->second++;
            }
        }
        
    }
    return distribution;
}

std::unordered_map<int64_t, int64_t> getOutputDistribution2(Blockchain &chain, int start, int stop) {
    auto mapFunc = [](const BlockRange &segment) {
        std::unordered_map<int64_t, int64_t> distribution;
        for (auto block : segment) {
            RANGES_FOR(auto tx, block) {
                RANGES_FOR(auto output, tx.outputs()) {
                    auto value = output.getValue();
                    value &= ~0xFF;
                    auto it = distribution.insert(std::make_pair(value, 0));
                    it.first->second++;
                }
            }
        }
        return distribution;
    };
    
    auto reduceFunc = [] (std::unordered_map<int64_t, int64_t> &map1, std::unordered_map<int64_t, int64_t> &map2) -> std::unordered_map<int64_t, int64_t> & {
        for (auto &pair : map2) {
            auto res = map1.insert(pair);
            if (!res.second) {
                res.first->second += pair.second;
            }
        }
        return map1;
    };
    
    return chain[{start, stop}].mapReduce<std::unordered_map<int64_t, int64_t>>(mapFunc, reduceFunc);
}

int64_t maxValOutput1(Blockchain &chain, int start, int stop) {
    int64_t maxValue = 0;
    for (int height = start; height < stop; height++) {
        auto block = chain[height];
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                maxValue = std::max(maxValue, output.getValue());
            }
        }
    }
    return maxValue;
}

int64_t maxValOutput2(Blockchain &chain, int start, int stop) {
    auto extract = [](const Transaction &tx) {
        int64_t maxValue = 0;
        for(auto output : tx.outputs()) {
            maxValue = std::max(output.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](int64_t &a, int64_t &b) -> int64_t & { a = std::max(a,b); return a; };
    
    return chain[{start, stop}].mapReduce<int64_t>(extract, combine);
}
