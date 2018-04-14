//
//  blockchain_heuristics.cpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#include "blockchain_heuristics.hpp"
#include "tx_identification.hpp"
#include <blocksci/chain/blockchain.hpp>

namespace blocksci { namespace heuristics {
    std::vector<Transaction> getDeanonTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return isDeanonTx(tx);
        });
    }
    
    std::vector<Transaction> getChangeOverTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return isChangeOverTx(tx);
        });
    }
    
    std::vector<Transaction> getKeysetChangeTxes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return containsKeysetChange(tx);
        });
    }
    
    std::vector<Transaction> getCoinjoinTransactions(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock)  {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return heuristics::isCoinjoin(tx);
        });
    }
    
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(Blockchain &chain, uint64_t minBaseFee, double percentageFee, size_t maxDepth)  {
        
        auto mapFunc = [&](const std::vector<Block> &segment) {
            std::vector<Transaction> skipped;
            std::vector<Transaction> txes;
            for (auto &block : segment) {
                RANGES_FOR(auto tx, block) {
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
        
        return chain.mapReduce<RetType>(BlockHeight{0}, static_cast<BlockHeight>(chain.size()), mapFunc, reduceFunc);
    }
}}
