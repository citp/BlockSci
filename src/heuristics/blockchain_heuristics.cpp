//
//  blockchain_heuristics.cpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#include <blocksci/heuristics/blockchain_heuristics.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/chain/blockchain.hpp>

namespace blocksci { namespace heuristics {
    std::vector<Transaction> getDeanonTxes(BlockRange &chain) {
        return chain.filter([](const Transaction &tx) {
            return isDeanonTx(tx);
        });
    }
    
    std::vector<Transaction> getChangeOverTxes(BlockRange &chain) {
        return chain.filter([](const Transaction &tx) {
            return isChangeOverTx(tx);
        });
    }
    
    std::vector<Transaction> getKeysetChangeTxes(BlockRange &chain) {
        return chain.filter([](const Transaction &tx) {
            return containsKeysetChange(tx);
        });
    }
    
    std::vector<Transaction> getCoinjoinTransactions(BlockRange &chain)  {
        return chain.filter([](const Transaction &tx) {
            return heuristics::isCoinjoin(tx);
        });
    }
    
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(Blockchain &chain, int64_t minBaseFee, double percentageFee, size_t maxDepth)  {
        
        auto mapFunc = [&](const BlockRange &segment) {
            std::vector<Transaction> skipped;
            std::vector<Transaction> txes;
            for (auto block : segment) {
                for (auto tx : block) {
                    auto label = heuristics::isPossibleCoinjoin(tx, minBaseFee, percentageFee, maxDepth);
                    if (label == heuristics::CoinJoinResult::True) {
                        txes.push_back(tx);
                    } else if (label == heuristics::CoinJoinResult::Timeout) {
                        skipped.push_back(tx);
                    }
                }
            }
            return std::make_pair(txes, skipped);
        };
        
        using RetType = std::pair<std::vector<Transaction>, std::vector<Transaction>>;
        
        auto reduceFunc = [] (RetType &a, RetType &b) -> RetType & {
            a.first.insert(a.first.end(), b.first.begin(), b.first.end());
            a.second.insert(a.second.end(), b.second.begin(), b.second.end());
            return a;
        };
        
        return chain.mapReduce<RetType>(mapFunc, reduceFunc);
    }
}}
