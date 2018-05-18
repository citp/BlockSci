//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "performance.hpp"

#include <blocksci/blocksci.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/script.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/transaction_range.hpp>

#include <range/v3/all.hpp>

#include <numeric>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>

using namespace blocksci;

void maxOutput(Blockchain &chain);

template <class R, class F, class T>
std::vector<R> mapParallel(const std::vector<T> &input, F fn) {
    std::atomic<size_t> idx{0};
    size_t end = input.size();
    
    std::vector<R> results;
    results.resize(end);
    
    int num_cpus = std::thread::hardware_concurrency();
    std::vector<std::future<void>> futures(num_cpus);
    for (int cpu = 0; cpu != num_cpus; ++cpu) {
        futures[cpu] = std::async(
                                  std::launch::async,
                                  [cpu, &idx, end, &fn, &results, &input]() {
                                      for (;;) {
                                          size_t i = idx++;
                                          if (i >= end) break;
                                          results[i] = fn(input[i]);
                                      }
                                  }
                                  );
    }
    for (int cpu = 0; cpu != num_cpus; ++cpu) {
        futures[cpu].get();
    }
    return results;
}

std::vector<int64_t> getTotalSpentOfAges2(const Block &block, BlockHeight maxAge) {
    std::vector<int64_t> totals(static_cast<size_t>(maxAge));
    
    uint32_t newestTxNum = block.prevBlock().endTxIndex() - 1;
    auto inputs = block.allInputs()
    | ranges::view::remove_if([=](const Input &input) { return input.spentTxIndex() > newestTxNum; });
    RANGES_FOR(auto input, inputs) {
        std::cout << input.age() << std::endl;
        BlockHeight age = std::min(maxAge, input.age()) - BlockHeight{1};
        totals[static_cast<size_t>(age)] += input.getValue();
    }
    for (BlockHeight i{1}; i < maxAge; ++i) {
        auto age = static_cast<size_t>(maxAge - i);
        totals[age - 1] += totals[age];
    }
    return totals;
}

int main(int argc, const char * argv[]) {
//    assert(argc == 2);
    
    Blockchain chain(argv[1]);
    
    TransactionRange txRange{chain.getAccess(), 0, txCount(chain)};
    
    int count = 0;
    RANGES_FOR(auto tx, txRange) {
        if (tx.fee() > 10000000) {
            count++;
        }
    }
    
    std::cout << count << std::endl;
    return 0;

//
//    std::unordered_map<Address, std::string> tags;
//    tags[chain[1000][0].outputs()[0].getAddress()] = "TestTag";
//
//    auto tagged = cm.taggedClusters(tags);
//
//    for (auto taggedCluster : tagged) {
//        std::cout << taggedCluster.cluster.getTypeEquivSize() << std::endl;
//    }
//
//    auto test = getTransactionsIncludingOutput(chain, 170, 171, AddressType::MULTISIG);
//
    return 0;
//    auto outs = multi.getEquivAddresses(true);
    std::cout << "test" << std::endl;
//    chain.access->addressIndex->checkDb();
    
    auto a = Transaction(1000, chain.getAccess());
    uint256 hash = a.getHash();
    std::cout << "Tx:" << a << std::endl;
    std::cout << "Hash:" << hash.GetHex() << std::endl;
    auto b = Transaction(hash, chain.getAccess());
    std::cout << "Tx:" << b << std::endl;
    auto c = a.outputs()[0].getAddress();
    std::cout << "Output: " << a.outputs()[0] << std::endl;
    auto d = c.getOutputs();
    for (auto out : d) {
        std::cout << out << std::endl;
    }

//    for(auto block : chain) {
//        RANGES_FOR(auto tx, block) {
//            std::cout << tx << std::endl;
//            auto timestamp = tx.getTimeSeen();
//            if (timestamp) {
//                std::time_t time = std::chrono::system_clock::to_time_t(*timestamp);
//                std::tm timetm = *std::localtime(&time);
//                std::cout << "output : " << std::put_time(&timetm, "%c %Z") << "+"
//                << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp->time_since_epoch()).count() % 1000 << std::endl;
//            }
////            RANGES_FOR(auto output, tx.outputs()) {
////                auto outputs = output.getAddress().getOutputs();
////                auto it = std::find(outputs.begin(), outputs.end(), output);
////                assert(it != outputs.end());
////                std::cout << output << std::endl;
////            }
//        }
//    }
//
    return 0;
//    uint64_t count = 0;
//    RANGES_FOR(auto tx, chain.iterateTransactions(0, chain.size())) {
//        auto change = getChangeOutput(tx);
//        if (change != nullptr) {
//            count++;
//        }
//    }
//    
//    std::cout << count << "\n";
    
//    auto block = chain[100000];
//    auto tx = block[0];
//    std::cout << tx.getHash().GetHex() << "\n";
//    auto output = tx.outputs()[0];
//    std::cout << output.getAddress().getScript().toPrettyString() << "\n";
//
//    return 0;
//
//    auto mapFunc = [&chain](const std::vector<Block> &segment) {
//        std::vector<Transaction> txes;
//        for (auto &block : segment) {
//            RANGES_FOR(auto tx, block) {
//                if (blocksci::heuristics::isCoinjoin(tx)) {
//                    txes.push_back(tx);
//                }
//            }
//        }
//        return txes;
//    };
    
//    auto reduceFunc = [] (std::vector<Transaction> &vec1, std::vector<Transaction> &vec2) -> std::vector<Transaction> &{
//        vec1.insert( vec1.end(), vec2.begin(), vec2.end() );
//        return vec1;
//    };
//
//    auto cjtxes = chain.mapReduce<std::vector<Transaction>>(354416, 464270, mapFunc, reduceFunc);
//
//    cjtxes.erase(std::remove_if(cjtxes.begin(), cjtxes.end(), [](const Transaction &tx) {
//        return tx.inputCount() > 15;
//    }), cjtxes.end());
//
//    std::vector<Transaction> easyCJs;
//    easyCJs.reserve(cjtxes.size());
//    std::vector<Transaction> hardCJs;
//    hardCJs.reserve(cjtxes.size());
//    for (auto &tx : cjtxes) {
//        auto res = blocksci::heuristics::isCoinjoinExtra(tx, 10000, .01, 1000);
//        if (res == blocksci::heuristics::CoinJoinResult::True) {
//            easyCJs.push_back(tx);
//        } else if (res == blocksci::heuristics::CoinJoinResult::Timeout) {
//            hardCJs.push_back(tx);
//        }
//    }
//
//    std::cout << "There were " << easyCJs.size() << " easy CoinJoins and " << hardCJs.size() << " hard Coinjoins\n";
    
//    auto results = mapParallel<CoinJoinResult>(cjtxes, [](const Transaction &tx) {
//        return isCoinjoinExtra(tx, 10000, .01, 0);
//    });
//
//    std::ofstream outputFile ("output.txt");
//    for (size_t i = 0; i < cjtxes.size(); i++) {
//        if (results[i]) {
//            outputFile << cjtxes[i].txNum << " ";
//        }
//    }
//    auto extract = [&](const Transaction &tx) {
//        uint64_t totalVal = 0;
//        for (auto &input : tx.inputs()) {
//            auto out = input.getSpentOutput();
//            if (input.sequenceNum != 4294967295) {
//                totalVal++;
//            }
//        }
//        return totalVal;
//    };
//    
//    
//    int64_t totalVal = chain.mapReduceTransactions(0, chain.size(), extract, [](int64_t a,int64_t b) {return a + b;}, int64_t{0});
//    
//    
//    std::cout << totalVal << "\n";
    
//    return 0;

//    auto tx = Transaction::txWithIndex(235179045);
//    return isCoinjoinExtra(tx);
    
//    auto txes = getTransactionIncludingOutput(chain, txnouttype::NULL_DATA);
//    for (auto &tx : txes) {
//        std::cout << tx << "\n";
//        std::cout << tx.getHash().GetHex() << "\n";
//        for (auto &output : tx.outputs) {
//            std::cout << output << "\n";
//            if (output.getType() == txnouttype::NULL_DATA) {
//                auto address = output.getAddress();
//                auto addressPointer = dynamic_cast<OpReturnAddress *>(address.get());
//                if (addressPointer) {
//                    std::cout << addressPointer->data;
//                }
//                
//            }
//        }
//        std::cout << "\n\n";
//    }
//    
    
//    auto begin = std::chrono::steady_clock::now();
//    int k = 0;
//    RANGES_FOR(auto tx, chain.iterateTransactions(0, chain.size())) {
//        k++;
//    }
//    auto endTime = std::chrono::steady_clock::now();
//    auto timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
    
//    begin = std::chrono::steady_clock::now();
//    int cmd = 0;
//    while (cmd != -1){
//        std::cout << "Tell me which block\n";
//        std::cin >> cmd;
//        std::cout << chain[cmd].getString() << std::endl;
//        
//    }
//    int j = 0;
//    for (auto &block : chain) {
//    for (int i = 700000; i < 717961; i++) {
//        auto block = chain[i];
//        std::cout << block.getString() << std::endl;
//        RANGES_FOR(auto tx, block) {
//            std::cout << tx.toString() << std::endl;
//            for (auto input : tx.inputs()) {
//                std::cout << input.toString() << std::endl;
//                std::cout << input.getAddress().getScript().toPrettyString() << std::endl;
//            }
//            for (auto output : tx.outputs()) {
//                std::cout << output.toString() << std::endl;
//                std::cout << output.getAddress().getScript().toPrettyString() << std::endl;
//            }
//        }
//    }
//    uint32_t maxSize = 0;
//    auto endTime = std::chrono::steady_clock::now();
//    auto timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
//    std::cout << maxSize << "\n";
//
//
//    begin = std::chrono::steady_clock::now();
//    RANGES_FOR(auto block, chain) {
//        RANGES_FOR(auto tx, block) {
//            maxSize = std::max(maxSize, tx.sizeBytes());
//        }
//    }
//
//    endTime = std::chrono::steady_clock::now();
//    timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
//    std::cout << maxSize << "\n";
//    begin = std::chrono::steady_clock::now();
//    unsigned int maxSize = 0;
//    for (auto &block : chain) {
//        RANGES_FOR(auto tx, block) {
//            maxSize = std::max(maxSize, tx.sizeBytes());
//        }
//    }
//    endTime = std::chrono::steady_clock::now();
//    timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
//    std::cout << maxSize << "\n";
//    
//    begin = std::chrono::steady_clock::now();
//    for (const auto &tx : chain.iterateTransactions(0, chain.size())) {
//        maxSize = std::max(maxSize, tx.sizeBytes());
//    }
//    
//    endTime = std::chrono::steady_clock::now();
//    timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
//    std::cout << maxSize << "\n";
//
//    
//    auto extract1 = [](const Transaction &tx) {
//        int64_t maxValue = 0;
//        for (auto &output : tx.outputs()) {
//            maxValue = std::max(maxValue, output.getValue());
//        }
//        return tx.sizeBytes();
//    };
//    auto combine = [](uint32_t a, uint32_t b) { return std::max(a,b); };
//    
//    begin = std::chrono::steady_clock::now();
//    
//    maxSize = chain.mapReduceTransactions(0, chain.size(), extract1, combine, 0u);
//    
//    endTime = std::chrono::steady_clock::now();
//    timeSecs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - begin).count() / 1000000.0;
//    std::cout << timeSecs << "\n";
//    std::cout << maxSize << "\n";
//    

    
    return 0;
    
    
//    auto count = DataAccess::Instance().addressCount<txnouttype::TX_PUBKEYHASH>();
//    for (uint32_t i = 1; i <= count; i++) {
//        auto address = Address(i, txnouttype::TX_PUBKEYHASH);
//        std:: cout << address << "\n";
//        auto outputs = address.getOutputTransactions();
//        for (auto &addTx : outputs) {
//            std::cout << addTx.getHashString() << " " << addTx << "\n";
//            
//        }
//    }


    
//    auto addCount = DataAccess::Instance().addressCount<txnouttype::TX_PUBKEYHASH>();
//    for (uint32_t i = 1; i <= addCount; i++) {
//        auto address = Address(i, txnouttype::TX_PUBKEYHASH);
//        uint32_t index = std::numeric_limits<uint32_t>::max();
//        auto outputs = address.getOutputTransactions();
//        for (auto &addTx : outputs) {
//            if (addTx.txNum < index) {
//                index = addTx.txNum;
//            }
//        }
//        if (index != address.getFirstTxNum()) {
//            std::cout << i << "\n";
//        }
//        assert (index == address.getFirstTxNum());
//    }
//    std::cout << addCount << std::endl;
//
//    RANGES_FOR(auto block, chain) {
//        RANGES_FOR(auto tx, block) {
//            RANGES_FOR(auto txout, tx.outputs()) {
//                auto address = txout.getAddress();
//                uint32_t index = tx.txNum;
//                auto outputs = address.getOutputTransactions();
//                for (auto &addTx : outputs) {
//                    if (addTx.txNum < index) {
//                        index = addTx.txNum;
//                    }
//                }
//                assert (index == address.getFirstTxNum());
//            }
//        }
//    }
    
//    auto address = chain[164676][14].outputs[1].getAddress();
//    auto addressData = boost::get<blocksci::ScriptData<blocksci::txnouttype::NONSTANDARD>>(address.getExtraData());
//    std::cout << addressData.inputData->length << "\n";
//    std::cout << static_cast<const void*>(addressData.inputData) << "\n";
//    std::cout << static_cast<const void*>(&addressData.inputData->data[0]) << "\n";
//    std::cout << addressData.outputData->length << "\n";
//    std::cout << static_cast<const void*>(addressData.outputData) << "\n";
//    std::cout << static_cast<const void*>(&addressData.outputData->data[0]) << "\n";
//    std::cout << address << "\n";
//    std::cout << chain[114][2].outputs[1] << std::endl;
    
//    auto start = 0;
//    auto end = chain.size();
    
//    auto dist = timeFunc("outputDist", [&] { return chain.getOutputDistribution(0, 150000);});
    
//    auto &block = chain[0];
//    RANGES_FOR(auto tx, block) {
//        RANGES_FOR(auto output, tx.outputs()) {
//            std::cout << output.toAddressString() << std::endl;
//        }
//    }

//    std::cout << Address(0, txnouttype::TX_PUBKEYHASH) << "\n";
//    Address add(0, txnouttype::TX_MULTISIG);
//    auto outputs = add.getOutputs();
//    std::cout << add << "\n";
//    for (auto output : outputs) {
//        std::cout << *output << "\n";
//    }
    
//    auto txes = add.getOutputTransactions();
//    RANGES_FOR(auto block, chain) {
//        RANGES_FOR(auto tx, block) {
////            std::cout << tx.getHashString() << " " << tx << "\n";
////            auto change = tx.getChangeOutput();
////            if (change) {
////                std::cout << "change " << *change << "\n\n";
////            }
//            
////            std::cout << "Spent Outputs\n";
////            for (auto &output : tx.spentOutputs()) {
////                std::cout << output << "\n";
////            }
//            std::cout << "Outputs\n";
//            for (auto &output : tx.outputs) {
//                std::cout << output << "\n";
//                std::cout << output.getAddress().getFirstTxNum() << "\n";
//            }
//        }
//        if (block.height == 50) {
//            break;
//        }
//    }
   // std::cout << Transaction::txAtIndex(100000) << "\n";
    
//    auto addr = getAddressFromString("1HWqMzw1jfpXb3xyuUZ4uWXY4tqL2cW47J");
//    if (addr) {
//        std::cout << *addr << "\n";
//    } else {
//        std::cout << "Not found \n";
//    }
    
    
//    RANGES_FOR(auto block, chain) {
//        RANGES_FOR(auto tx, block) {
//            RANGES_FOR(auto output, tx.outputs()) {
//                std::cout << output << "\n";
//            }
//        }
//    }
  

    return 0;
}
    
void maxOutput(Blockchain &chain) {
    int64_t maxValue = 0;
    uint32_t txNum = 0;
    uint32_t curTx = 0;
    for(auto block : chain) {
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                if (output.getValue() > maxValue) {
                    maxValue = output.getValue();
                    txNum = curTx;
                }
            }
            curTx++;
        }
    }
    std::cout << "Max value is " << maxValue << " in tx " << txNum << std::endl;
    
    std::cout << "Tx has hash " << Transaction(txNum, chain.getAccess()).getHash().GetHex() << std::endl;
}

//google::dense_hash_map<uint32_t, uint32_t> getAddressDistribution(Blockchain &chain, int start, int stop) {
//    auto mapFunc = [](const std::vector<Block> &segment) {
//        google::dense_hash_map<uint32_t, uint32_t> distribution;
//        distribution.set_empty_key(std::numeric_limits<uint32_t>::max() - 5);
//        for (auto &block : segment) {
//            RANGES_FOR(auto tx, block) {
//                RANGES_FOR(auto output, tx.outputs()) {
//                    auto it = distribution.insert(std::make_pair(output.getSpendingTxIndex(), 0));
//                    it.first->second++;
//                }
//            }
//        }
//        return distribution;
//    };
//
//    auto reduceFunc = [] (google::dense_hash_map<uint32_t, uint32_t> &map1, google::dense_hash_map<uint32_t, uint32_t> &map2) -> google::dense_hash_map<uint32_t, uint32_t> & {
//        for (auto &pair : map2) {
//            auto res = map1.insert(pair);
//            if (!res.second) {
//                res.first->second += pair.second;
//            }
//        }
//        return map1;
//    };
//
//    return chain.mapReduce<google::dense_hash_map<uint32_t, uint32_t>>(start, stop, mapFunc, reduceFunc);
//}

/*
google::dense_hash_map<uint32_t, uint32_t> getMultisigAddressDistribution(Blockchain &chain, int start, int stop) {
    auto mapFunc = [](std::vector<Block> &segment) {
        google::dense_hash_map<uint32_t, uint32_t> distribution;
        distribution.set_empty_key(std::numeric_limits<uint32_t>::max() - 5);
        for (auto &block : segment) {
            for (uint32_t i = 0; i < block.numTxes; i++) {
                if (Transaction::txContainsOutputOfType(block.firstTxIndex + i, blocksci::txnouttype::TX_MULTISIG)) {
                    auto tx = block[i];
                    for(auto output : tx.outputs) {
                        if (output.getType() == TX_MULTISIG) {
                            auto data = boost::get<const ScriptData<TX_MULTISIG> *>(output.getAddress().getExtraData());
                            for (uint16_t i = 0; i < data->n; i++) {
                                auto it = distribution.insert(std::make_pair(data->addressArray[i], 0));
                                it.first->second++;
                            }
                        }
                    }
                }
            }
            //            RANGES_FOR(auto tx, block) {
            //                if (tx.containsOutputOfType(txnouttype::TX_MULTISIG)) {
            //                    for(auto output : tx.expandedOutputs()) {
            //                        if (output.getType() == txnouttype::TX_MULTISIG) {
            //                            auto data = boost::get<MultisigData>(output.extraData);
            //                            for (uint16_t i = 0; i < data.n; i++) {
            //                                auto it = distribution.insert(std::make_pair(data.addresses[i], 0));
            //                                it.first->second++;
            //                            }
            //
            //
            //                        }
            //
            //                    }
            //                }
            //
            //            }
        }
        return distribution;
    };
    
    auto reduceFunc = [] (google::dense_hash_map<uint32_t, uint32_t> &map1, google::dense_hash_map<uint32_t, uint32_t> &map2) {
        for (auto &pair : map2) {
            auto res = map1.insert(pair);
            if (!res.second) {
                res.first->second += pair.second;
            }
        }
    };
    
    return chain.mapReduceBlockRanges(start, stop, mapFunc, reduceFunc);
}
*/
