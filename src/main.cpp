//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/util/data_access.hpp>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/chain/transaction_summary.hpp>
#include <blocksci/address/address_types.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <gperftools/profiler.h>

#include <range/v3/view/slice.hpp>
#include <range/v3/algorithm/count_if.hpp>

#include <google/sparse_hash_map>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <future>
#include <numeric>

#include <fstream>

using namespace blocksci;

google::sparse_hash_map<uint64_t, uint64_t> getOutputDistribution(Blockchain &chain, uint32_t start, uint32_t stop);
uint64_t calculateMaxOutputSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end);
uint64_t calculateMaxOutputMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop);
uint64_t calculateMaxInputSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end);
uint64_t calculateMaxInputMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop);
uint64_t calculateMaxFeeSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end);
uint64_t calculateMaxFeeMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop);
uint32_t calculateNonzeroLocktimeSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end);
uint32_t calculateNonzeroLocktimeMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop);
TransactionSummary simpleStatsSingleThreaded(Blockchain &chain, uint32_t start, uint32_t stop);
TransactionSummary simpleStatsMultiThreaded(Blockchain &chain, uint32_t start, uint32_t stop);

uint64_t calculateNonzeroLocktimeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes);
uint64_t calculateMaxFeeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes);

template <typename Func, typename... Args>
auto timeFunc(std::string name, Func func, Args && ...args) -> decltype(func(args...));

int main(int argc, const char * argv[]) {
    Blockchain chain(argv[1]);
    
    uint32_t startBlock = 0;
    uint32_t endBlock = atoi(argv[2]);
    if (endBlock == 0) {
        endBlock = chain.size();
    }
    
    std::cout << "Running performance tests up to block " << endBlock << "\n";
    
   std::cout << "Heating up cache\n";
   calculateMaxFeeMultithreaded(chain, 0, endBlock);
   std::cout << "Finished heating up cache\n";

   // uint32_t maxTxNum = chain[endBlock - 1].endTxIndex();
   // std::vector<uint32_t> indexes(maxTxNum);
   // std::iota(indexes.begin(), indexes.end(), 0);
   // std::random_shuffle(indexes.begin(), indexes.end());
   // ProfilerStart("prof.out");
   //  // 
   //  timeFunc("calculateNonzeroLocktimeSingleThreaded", calculateNonzeroLocktimeSingleThreaded, chain, startBlock, endBlock);
   // auto maxOutputA = timeFunc("calculateMaxOutputSingleThreaded", calculateMaxOutputSingleThreaded, chain, startBlock, endBlock);
   // auto maxOutputB = timeFunc("calculateMaxOutputSingleThreaded2", calculateMaxOutputSingleThreaded2, chain, startBlock, endBlock);
   // auto maxOutputC = timeFunc("calculateMaxOutputSingleThreaded3", calculateMaxOutputSingleThreaded3, chain, startBlock, endBlock);
   // std::cout << maxOutputA << " " << maxOutputB << " " << maxOutputC << std::endl;
   // ProfilerStart("prof.out");
   // auto a = timeFunc("calculateUnspentCount1", calculateUnspentCount1, chain, startBlock, endBlock);
   // auto b = timeFunc("calculateUnspentCount2", calculateUnspentCount2, chain, startBlock, endBlock);
   // auto a = calculateNonzeroLocktimeSingleThreaded(chain, startBlock, endBlock);
   // auto b = calculateMaxOutputSingleThreaded(chain, startBlock, endBlock);
   // auto c = calculateMaxInputSingleThreaded(chain, startBlock, endBlock);
   // auto d = calculateMaxFeeSingleThreaded(chain, startBlock, endBlock);
   // ProfilerStop();
   // std::cout << a << " " << b << std::endl;
   // return 0;
   // ProfilerStart("prof.out");
    auto maxSize1 = timeFunc("calculateNonzeroLocktimeSingleThreaded", calculateNonzeroLocktimeSingleThreaded, chain, startBlock, endBlock);
    auto maxSize2 = timeFunc("calculateNonzeroLocktimeMultithreaded", calculateNonzeroLocktimeMultithreaded, chain, startBlock, endBlock);
    // auto maxSize3 = timeFunc("calculateNonzeroLocktimeRandom", calculateNonzeroLocktimeRandom, chain, indexes);
    auto maxOutput1 = timeFunc("calculateMaxOutputSingleThreaded", calculateMaxOutputSingleThreaded, chain, startBlock, endBlock);
    auto maxOutput2 = timeFunc("calculateMaxOutputMultithreaded", calculateMaxOutputMultithreaded, chain, startBlock, endBlock);
    auto maxInput1 = timeFunc("calculateMaxInputSingleThreaded", calculateMaxInputSingleThreaded, chain, startBlock, endBlock);
    auto maxInput2 = timeFunc("calculateMaxInputMultithreaded", calculateMaxInputMultithreaded, chain, startBlock, endBlock);
    auto maxFee1 = timeFunc("calculateMaxFeeSingleThreaded", calculateMaxFeeSingleThreaded, chain, startBlock, endBlock);
    auto maxFee2 = timeFunc("calculateMaxFeeMultithreaded", calculateMaxFeeMultithreaded, chain, startBlock, endBlock);
    // auto maxFee3 = timeFunc("calculateMaxFeeRandom", calculateMaxFeeRandom, chain, indexes);
    // auto simpleStats1 = timeFunc("simpleStatsSingleThreaded", simpleStatsSingleThreaded, chain, startBlock, endBlock);
    // auto simpleStats2 = timeFunc("simpleStatsMultiThreaded", simpleStatsMultiThreaded, chain, startBlock, endBlock);
    // auto outputDistribution = timeFunc("getOutputDistribution", getOutputDistribution, chain, startBlock, endBlock);
    
    // auto opreturns = timeFunc("getTransactionIncludingOutput", getTransactionIncludingOutput, chain, startBlock, endBlock, AddressType::Enum::NULL_DATA);
    
   // ProfilerStop();

    std::cout << "Nonzero Locktime = (" << maxSize1 << ", " << maxSize2 << ")\n";
    std::cout << "Max Output = (" << maxOutput1 << ", " << maxOutput2 << ")\n";
    std::cout << "Max Input = (" << maxInput1 << ", " << maxInput2 << ")\n";
    std::cout << "Max Fee = (" << maxFee1 << ", " << maxFee2 << ")\n";
    return 0;
}
    
google::sparse_hash_map<uint64_t, uint64_t> getOutputDistribution(Blockchain &chain, uint32_t start, uint32_t stop) {
    using DistMap = google::sparse_hash_map<uint64_t, uint64_t>;

    auto mapFunc = [&](const std::vector<Block> &segment) {
        DistMap distribution;
        distribution.resize(10000000);
        for (auto &block : segment) {
            RANGES_FOR (auto tx, block) {
                RANGES_FOR(auto output, tx.outputs()) {
                    auto value = output.getValue();
                    value &= ~0x3FFF;
                    auto it = distribution.insert(std::make_pair(value, 0));
                    it.first->second++;
                }
            }
        }
        return distribution;
    };
    
    auto reduceFunc = [] (DistMap &map1, DistMap &map2) -> DistMap & {
        for (auto &pair : map2) {
            auto res = map1.insert(pair);
            if (!res.second) {
                res.first->second += pair.second;
            }
        }
        return map1;
    };
    
    return chain.mapReduce<DistMap>(start, stop, mapFunc, reduceFunc);
}

uint64_t calculateMaxOutputSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end) {
    uint64_t curMax = 0;
    RANGES_FOR (auto block, chain[{start, end}]) {
        RANGES_FOR (auto tx, block) {
            RANGES_FOR (auto output, tx.rawOutputs()) {
                curMax = std::max(curMax, output.getValue());
            }
        }
    }
    return curMax;
}

uint64_t calculateMaxOutputMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) {
        uint64_t maxValue = 0;
        RANGES_FOR (auto output, tx.rawOutputs()) {
            maxValue = std::max(output.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](uint64_t &a, uint64_t &b) -> uint64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<uint64_t>(start, stop, extract, combine);
}

uint64_t calculateMaxInputSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end) {
    uint64_t curMax = 0;
    RANGES_FOR (auto block, chain[{start, end}]) {
        RANGES_FOR (auto tx, block) {
            RANGES_FOR (auto input, tx.rawInputs()) {
                curMax = std::max(curMax, input.getValue());
            }
        }
    }
    return curMax;
}

uint64_t calculateMaxInputMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [&](const Transaction &tx) {
        uint64_t maxValue = 0;
        RANGES_FOR (auto input, tx.rawInputs()) {
            maxValue = std::max(input.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](uint64_t &a, uint64_t &b) -> uint64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<uint64_t>(start, stop, extract, combine);
}

uint64_t calculateMaxFeeSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end) {
    uint64_t curMax = 0;
    RANGES_FOR (auto block, chain[{start, end}]) {
        RANGES_FOR (auto tx, block) {
            curMax = std::max(curMax, fee(tx));
        }
    }
    return curMax;
}

uint64_t calculateMaxFeeMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) {
        return fee(tx);
    };
    
    auto combine = [](uint64_t &a, uint64_t &b) -> uint64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<uint64_t>(start, stop, extract, combine);
}

uint64_t calculateMaxFeeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes) {
    uint64_t maxValue = 0;
    for (auto index : indexes) {
        auto tx = Transaction(index, chain.getAccess());
        maxValue = std::max(maxValue, fee(tx));
    }
    return maxValue;
}

uint32_t calculateNonzeroLocktimeSingleThreaded(Blockchain &chain, uint32_t start, uint32_t end) {
    uint32_t count = 0;
    RANGES_FOR (auto block, chain[{start, end}]) {
        RANGES_FOR (auto tx, block) {
            count += tx.locktime() > 0;
        }
    }
    return count;
}

uint32_t calculateNonzeroLocktimeMultithreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) {
        return static_cast<uint32_t>(tx.locktime() > 0);
    };
    
    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a += b; return a; };
    
    return chain.mapReduce<uint32_t>(start, stop, extract, combine);
}

uint64_t calculateNonzeroLocktimeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes) {
    uint64_t nonzeroCount = 0;
    for (auto index : indexes) {
        auto tx = Transaction(index, chain.getAccess());
        nonzeroCount += static_cast<uint32_t>(tx.locktime() > 0);
    }
    return nonzeroCount;
}


TransactionSummary simpleStatsMultiThreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    auto extract = [](const Transaction &tx) {
        TransactionSummary summary{};
        summary += tx;
        return summary;
        
    };
    
    auto combine = [](TransactionSummary &a, TransactionSummary &b) -> TransactionSummary & { a += b; return a; };
    
    return chain.mapReduce<TransactionSummary>(start, stop, extract, combine);
}

TransactionSummary simpleStatsSingleThreaded(Blockchain &chain, uint32_t start, uint32_t stop) {
    TransactionSummary sum;
    RANGES_FOR (auto block, chain[{start, stop}]) {
        RANGES_FOR (auto tx, block) {
            sum += tx;
        }
    }
    return sum;
}


template <typename Func, typename... Args>
auto timeFunc(std::string name, Func func, Args && ...args) -> decltype(func(args...)) {
    auto begin = std::chrono::steady_clock::now();
    auto ret = func(std::forward<Args>(args)...);
    auto endTime = std::chrono::steady_clock::now();
    
    double timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
    std::cout << "Time in secs for " << name << ": " << timeSecs << std::endl;
    
    return ret;
}
