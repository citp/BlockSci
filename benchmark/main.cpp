//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/blocksci.hpp>
#include <range/v3/view/slice.hpp>
#include <clipp.h>

#include <numeric>
#include <iostream>

using namespace blocksci;

int64_t countBlocks(BlockRange &chain);
int64_t calculateMaxOutputSingleThreaded(BlockRange &chain);
int64_t calculateMaxOutputMultithreaded(BlockRange &chain);
int64_t calculateMaxInputSingleThreaded(BlockRange &chain);
int64_t calculateMaxInputMultithreaded(BlockRange &chain);
int64_t calculateMaxFeeSingleThreaded(BlockRange &chain);
int64_t calculateMaxFeeMultithreaded(BlockRange &chain);
uint32_t calculateNonzeroLocktimeSingleThreaded(BlockRange &chain);
uint32_t calculateNonzeroLocktimeMultithreaded(BlockRange &chain);
uint32_t calculateVersionGreaterOneSingleThreaded(BlockRange &chain);
uint32_t calculateVersionGreaterOneMultithreaded(BlockRange &chain);
uint32_t calculateUniqueLocktimeChangeSingleThreaded(BlockRange &chain);
uint32_t calculateUniqueLocktimeChangeMultithreaded(BlockRange &chain);
uint32_t calculateZeroConfOutputSingleThreaded(BlockRange &chain);
uint32_t calculateZeroConfOutputMultithreaded(BlockRange &chain);

int64_t calculateSatoshiDiceTotalOutputValue(BlockRange &chain, uint32_t addressNum, AddressType::Enum type);

uint32_t calculateNonzeroLocktimeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes);
int64_t calculateMaxFeeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes);

template <typename Func, typename... Args>
auto timeFunc(std::string name, Func func, uint32_t iterations, Args&& ...args) -> decltype(func(args...));

int main(int argc, char * argv[]) {
    bool includeRandom = false;
    bool includeTraversal = false;
    std::string configLocation;
    int endBlock = 0;
    uint32_t iterations = 1;

    auto cli = (
        clipp::value("config file location", configLocation),
        clipp::option("-r", "--with-random").set(includeRandom).doc("Include random order benchmarks"),
        clipp::option("-t", "--with-traversal").set(includeTraversal).doc("Include graph traversal benchmarks"),
        clipp::option("-m", "--max-block") & clipp::value("Run benchmark up to the given block", endBlock),
        clipp::option("-i", "--iterations") & clipp::value("Number of iterations for each benchmark", iterations)
    );
    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
        std::cout << "Invalid command line parameter\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }

    Blockchain chain(configLocation, endBlock);
    
    std::cout << "Heating up cache." << std::endl;

    timeFunc("loadingTxData", calculateMaxFeeMultithreaded, 1, chain);
    timeFunc("loadingVersionNo", calculateVersionGreaterOneSingleThreaded, 1, chain);

    auto totalBlocks = timeFunc("countBlocks", countBlocks, 1, chain);
    std::cout << "Running benchmark over " << totalBlocks << " blocks." << std::endl;

    std::cout << std::endl << "Benchmarks:" << std::endl;

    // Sequential transaction graph iteration
    auto locktime1 = timeFunc("nonzeroLocktimeSingleThreaded", calculateNonzeroLocktimeSingleThreaded, iterations, chain);
    auto locktime2 = timeFunc("nonzeroLocktimeMultithreaded", calculateNonzeroLocktimeMultithreaded, iterations, chain);
    auto maxOutput1 = timeFunc("maxOutputSingleThreaded", calculateMaxOutputSingleThreaded, iterations, chain);
    auto maxOutput2 = timeFunc("maxOutputMultithreaded", calculateMaxOutputMultithreaded, iterations, chain);
    auto maxInput1 = timeFunc("maxInputSingleThreaded", calculateMaxInputSingleThreaded, iterations, chain);
    auto maxInput2 = timeFunc("maxInputMultithreaded", calculateMaxInputMultithreaded, iterations, chain);
    auto maxFee1 = timeFunc("maxFeeSingleThreaded", calculateMaxFeeSingleThreaded, iterations, chain);
    auto maxFee2 = timeFunc("maxFeeMultithreaded", calculateMaxFeeMultithreaded, iterations, chain);

    auto version1 = timeFunc("versionGreaterOneSingleThreaded", calculateVersionGreaterOneSingleThreaded, iterations, chain);
    auto version2 = timeFunc("versionGreaterOneMultithreaded", calculateVersionGreaterOneMultithreaded, iterations, chain);

    // Graph traversal queries
    uint32_t uniqueLocktimeSingle = 0;
    uint32_t uniqueLocktimeMulti = 0;
    uint32_t zeroconfSingle = 0;
    uint32_t zeroconfMulti = 0;

    if(includeTraversal) {
        uniqueLocktimeSingle = timeFunc("uniqueLocktimeChangeSingleThreaded", calculateUniqueLocktimeChangeSingleThreaded, iterations, chain);
        uniqueLocktimeMulti = timeFunc("uniqueLocktimeChangeMultithreaded", calculateUniqueLocktimeChangeMultithreaded, iterations, chain);
        zeroconfSingle = timeFunc("zeroConfOutputSingleThreaded", calculateZeroConfOutputSingleThreaded, iterations, chain);
        zeroconfMulti = timeFunc("zeroConfOutputMultithreaded", calculateZeroConfOutputMultithreaded, iterations, chain);
    }

    int64_t maxSatoshiDiceOutput = -1;
    auto satoshiDiceAddress = getAddressFromString("1dice97ECuByXAvqXpaYzSaQuPVvrtmz6", chain.getAccess());
    if(satoshiDiceAddress) {
        maxSatoshiDiceOutput = timeFunc("satoshiDiceTotalOutputValueSingleThreaded", calculateSatoshiDiceTotalOutputValue, iterations, chain, satoshiDiceAddress->scriptNum, satoshiDiceAddress->type);
    }

    if (includeRandom) {
        uint32_t maxTxNum = chain[totalBlocks - 1].endTxIndex();
        std::vector<uint32_t> indexes(maxTxNum);
        std::iota(indexes.begin(), indexes.end(), 0);
        std::random_shuffle(indexes.begin(), indexes.end());

        timeFunc("maxFeeRandom", calculateMaxFeeRandom, iterations, chain, indexes);
        timeFunc("nonzeroLocktimeRandom", calculateNonzeroLocktimeRandom, iterations, chain, indexes);
    }

    // Print results
    std::cout << std::endl << "Results:" << std::endl;;
    std::cout << "Nonzero Locktime = (" << locktime1 << ", " << locktime2 << ")" << std::endl;
    std::cout << "Max Output = (" << maxOutput1 << ", " << maxOutput2 << ")" << std::endl;
    std::cout << "Max Input = (" << maxInput1 << ", " << maxInput2 << ")" << std::endl;
    std::cout << "Max Fee = (" << maxFee1 << ", " << maxFee2 << ")" << std::endl;
    std::cout << "Version > 1 = (" << version1 << ", " << version2 << ")" << std::endl;
    if(maxSatoshiDiceOutput >= 0) {
        std::cout << "SatoshiDice Output Value = (" << maxSatoshiDiceOutput << ")" << std::endl;
    }
    if(includeTraversal) {
        std::cout << "Zeroconf Outputs = (" << zeroconfSingle << ", " << zeroconfMulti << ")" << std::endl;
        std::cout << "Unique Change = (" << uniqueLocktimeSingle << ", " << uniqueLocktimeMulti << ")" << std::endl;
    }
    return 0;
}

int64_t countBlocks(BlockRange &chain) {
    int64_t count = 0;
    for (auto block : chain) {
        count += 1;
    }
    return count;
}

int64_t calculateMaxOutputSingleThreaded(BlockRange &chain) {
    int64_t curMax = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            for (auto output : tx.outputs()) {
                curMax = std::max(curMax, output.getValue());
            }
        }
    }
    return curMax;
}

int64_t calculateMaxOutputMultithreaded(BlockRange &chain) {
    auto extract = [](const Transaction &tx) {
        int64_t maxValue = 0;
        for(auto output : tx.outputs()) {
            maxValue = std::max(output.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](int64_t &a, int64_t &b) -> int64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<int64_t>(extract, combine);
}

int64_t calculateMaxInputSingleThreaded(BlockRange &chain) {
    int64_t curMax = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            for (auto input : tx.inputs()) {
                curMax = std::max(curMax, input.getValue());
            }
        }
    }
    return curMax;
}

int64_t calculateMaxInputMultithreaded(BlockRange &chain) {
    auto extract = [&](const Transaction &tx) {
        int64_t maxValue = 0;
        for(auto input : tx.inputs()) {
            maxValue = std::max(input.getValue(), maxValue);
        }
        return maxValue;
    };
    
    auto combine = [](int64_t &a, int64_t &b) -> int64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<int64_t>(extract, combine);
}

int64_t calculateMaxFeeSingleThreaded(BlockRange &chain) {
    int64_t curMax = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            curMax = std::max(curMax, fee(tx));
        }
    }
    return curMax;
}

int64_t calculateMaxFeeMultithreaded(BlockRange &chain) {
    auto extract = [](const Transaction &tx) {
        return fee(tx);
    };
    
    auto combine = [](int64_t &a, int64_t &b) -> int64_t & { a = std::max(a,b); return a; };
    
    return chain.mapReduce<int64_t>(extract, combine);
}

int64_t calculateMaxFeeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes) {
    int64_t maxValue = 0;
    for (auto index : indexes) {
        auto tx = Transaction(index, chain.getAccess());
        maxValue = std::max(maxValue, fee(tx));
    }
    return maxValue;
}

uint32_t calculateNonzeroLocktimeSingleThreaded(BlockRange &chain) {
    uint32_t count = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            count += tx.locktime() > 0;
        }
    }
    return count;
}

uint32_t calculateNonzeroLocktimeMultithreaded(BlockRange &chain) {
    auto extract = [](const Transaction &tx) {
        return static_cast<uint32_t>(tx.locktime() > 0);
    };
    
    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a += b; return a; };
    
    return chain.mapReduce<uint32_t>(extract, combine);
}

uint32_t calculateVersionGreaterOneSingleThreaded(BlockRange &chain) {
    uint32_t count = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            count += tx.getVersion() > 1;
        }
    }
    return count;
}

uint32_t calculateVersionGreaterOneMultithreaded(BlockRange &chain) {
    auto extract = [](const Transaction &tx) {
        return static_cast<uint32_t>(tx.getVersion() > 1);
    };

    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a += b; return a; };

    return chain.mapReduce<uint32_t>(extract, combine);
}

uint32_t calculateNonzeroLocktimeRandom(Blockchain &chain, const std::vector<uint32_t> &indexes) {
    uint32_t nonzeroCount = 0;
    for (auto index : indexes) {
        auto tx = Transaction(index, chain.getAccess());
        nonzeroCount += static_cast<uint32_t>(tx.locktime() > 0);
    }
    return nonzeroCount;
}

uint32_t calculateZeroConfOutputSingleThreaded(BlockRange &chain) {
    uint32_t count = 0;
    for (auto block : chain) {
        RANGES_FOR(auto tx, block) {
            for(auto output : tx.outputs()) {
                if(output.isSpent()) {
                    auto spendingTx = output.getSpendingTx();
                    if(spendingTx->getBlockHeight() == block.height()) {
                        count += 1;
                    }
                }
            }
        }
    }
    return count;
}

uint32_t calculateZeroConfOutputMultithreaded(BlockRange &chain) {
    auto extract = [&](const Transaction &tx) {
        uint32_t count = 0;
        auto blockHeight = tx.getBlockHeight();
        for(auto output : tx.outputs()) {
            if(output.isSpent()) {
                auto spendingTx = output.getSpendingTx();
                if(spendingTx->getBlockHeight() == blockHeight) {
                    count += 1;
                }
            }
        }
        return count;
    };

    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a += b; return a; };

    return chain.mapReduce<uint32_t>(extract, combine);
}

int64_t calculateSatoshiDiceTotalOutputValue(BlockRange &chain, uint32_t addressNum, AddressType::Enum type) {
    auto address = Address{addressNum, type, chain.getAccess()};
    int64_t total = 0;
    RANGES_FOR(auto out, address.getOutputs()) {
        total += out.getValue();
    }
    return total;
}

uint32_t calculateUniqueLocktimeChangeSingleThreaded(BlockRange &chain) {
    uint32_t count = 0;
    for(auto block : chain) {
        RANGES_FOR(auto tx, block) {
            auto outputCount = 0;
            bool locktimeBehavior = tx.locktime() > 0;
            for(auto output : tx.outputs()) {
                if(output.isSpent()) {
                    auto spendingTx = output.getSpendingTx();
                    if((spendingTx->locktime() > 0) == locktimeBehavior) {
                        outputCount += 1;
                    }
                }
            }
            count += static_cast<uint32_t>(outputCount == 1);
        }
    }
    return count;
}

uint32_t calculateUniqueLocktimeChangeMultithreaded(BlockRange &chain) {
    auto extract = [](const Transaction &tx) {
        auto outputCount = 0;
        bool locktimeBehavior = tx.locktime() > 0;
        for(auto output : tx.outputs()) {
            if(output.isSpent()) {
                auto spendingTx = output.getSpendingTx();
                if((spendingTx->locktime() > 0) == locktimeBehavior) {
                    outputCount += 1;
                }
            }
        }
        return static_cast<uint32_t>(outputCount == 1);
    };

    auto combine = [](uint32_t &a, uint32_t &b) -> uint32_t & { a += b; return a; };

    return chain.mapReduce<uint32_t>(extract, combine);
}



template <typename Func, typename... Args>
auto timeFunc(std::string name, Func func, uint32_t iterations, Args&& ...args) -> decltype(func(args...)) {
    std::vector<double> times;
    int64_t ret = 0;

    std::cout << "Time in secs for " << name << ": ";
    for(uint32_t i = 0; i < iterations; ++i) {
        auto begin = std::chrono::steady_clock::now();
        ret = func(std::forward<Args>(args)...);
        auto endTime = std::chrono::steady_clock::now();
        double timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
        times.push_back(timeSecs);
    }

    // compute average over all measurements
    double average = accumulate(times.begin(), times.end(), 0.0)/times.size();
    std::cout << average;

    // print individual observations
    if(iterations > 1) {
        std::cout << " [";
        for(uint32_t i = 0; i < iterations; ++i) {
            std::cout << times[i];
            if (i < iterations - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    }
    std::cout << std::endl;
    
    return ret;
}
