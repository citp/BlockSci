//
//  blockchain.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "blockchain.hpp"

#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/index/address_output_range.hpp>

#include <range/v3/action/push_back.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/front.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>

#include <fstream>
#include <iostream>

namespace blocksci {
    // [start, end)
    std::vector<std::vector<Block>> segmentChain(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, unsigned int segmentCount) {
        auto lastTx = chain[endBlock - BlockHeight{1}].endTxIndex();
        auto firstTx = chain[startBlock].firstTxIndex();
        auto totalTxCount = lastTx - firstTx;
        double segmentSize = static_cast<double>(totalTxCount) / segmentCount;
        
        std::vector<std::vector<Block>> segments;
        auto it = chain.begin();
        std::advance(it, static_cast<int>(startBlock));
        auto chainEnd = chain.begin();
        std::advance(chainEnd, static_cast<int>(endBlock));
        while(lastTx - (*it).firstTxIndex() > segmentSize) {
            auto endIt = std::lower_bound(it, chainEnd, (*it).firstTxIndex() + segmentSize, [](const Block &block, uint32_t txNum) {
                return block.firstTxIndex() < txNum;
            });
            segments.emplace_back(it, endIt);
            it = endIt;
        }
        if (segments.size() == segmentCount) {
            segments.back().insert(segments.back().end(), it, chainEnd);
        } else {
            segments.emplace_back(it, chainEnd);
        }
        return segments;
    }
    
     std::vector<std::pair<int, int>> segmentChainIndexes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, unsigned int segmentCount) {
        auto lastTx = chain[endBlock - BlockHeight{1}].endTxIndex();
        auto firstTx = chain[startBlock].firstTxIndex();
        auto totalTxCount = lastTx - firstTx;
        double segmentSize = static_cast<double>(totalTxCount) / segmentCount;
        
        std::vector<std::pair<int, int>> segments;
        auto it = chain.begin();
        std::advance(it, static_cast<int>(startBlock));
        auto chainEnd = chain.begin();
        std::advance(chainEnd, static_cast<int>(endBlock));
        while(lastTx - (*it).firstTxIndex() > segmentSize) {
            auto endIt = std::lower_bound(it, chainEnd, (*it).firstTxIndex() + segmentSize, [](const Block &block, uint32_t txNum) {
                return block.firstTxIndex() < txNum;
            });
            auto startBlock = *it;
            auto endBlock = *endIt;
            segments.emplace_back(startBlock.height(), endBlock.height());
            it = endIt;
        }
        if (segments.size() == segmentCount) {
            segments.back().second = endBlock;
        } else {
            auto startBlock = *it;
            segments.emplace_back(startBlock.height(), endBlock);
        }
        return segments;
    }
    
    Blockchain::Blockchain(const std::string &dataDirectory) : Blockchain(DataConfiguration{dataDirectory, true, BlockHeight{0}}) {}
    
    Blockchain::Blockchain(const DataConfiguration &config) : access(config) {
        lastBlockHeight = access.chain.blockCount();
    }
    
    template<AddressType::Enum type>
    struct ScriptRangeFunctor {
        static ScriptRangeVariant f(Blockchain &chain) {
            return chain.scripts<type>();
        }
    };
    
    ScriptRangeVariant Blockchain::scripts(AddressType::Enum type) {
        static auto table = make_static_table<AddressType, ScriptRangeFunctor>(*this);
        auto index = static_cast<size_t>(type);
        return table.at(index);
    }
    
    uint32_t txCount(Blockchain &chain) {
        auto lastBlock = chain[chain.size() - BlockHeight{1}];
        return lastBlock.endTxIndex();
    }
    
    std::vector<Block> filter(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Block &tx)> testFunc)  {
        auto mapFunc = [&testFunc](const std::vector<Block> &segment) -> std::vector<Block> {
            return segment | ranges::view::filter(testFunc) | ranges::to_vector;
        };
        
        auto reduceFunc = [] (std::vector<Block> &vec1, std::vector<Block> &vec2) -> std::vector<Block> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        return chain.mapReduce<std::vector<Block>>(startBlock, endBlock, mapFunc, reduceFunc);
    }
    
    std::vector<Transaction> filter(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Transaction &tx)> testFunc)  {
        auto mapFunc = [&testFunc](const std::vector<Block> &segment) -> std::vector<Transaction> {
            std::vector<Transaction> txes;
            for (auto &block : segment) {
                txes |= ranges::action::push_back(block | ranges::view::filter(testFunc));
            }
            return txes;
        };
        
        auto reduceFunc = [] (std::vector<Transaction> &vec1, std::vector<Transaction> &vec2) -> std::vector<Transaction> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        return chain.mapReduce<std::vector<Transaction>>(startBlock, endBlock, mapFunc, reduceFunc);
    }
    
    std::vector<Transaction> getTransactionIncludingOutput(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, AddressType::Enum type) {
        return filter(chain, startBlock, endBlock, [type](const Transaction &tx) {
            return ranges::any_of(tx.outputs(), [=](const Output &output) {
                return output.getType() == type;
            });
        });
    }
    
    std::map<uint64_t, Address> mostValuableAddresses(Blockchain &chain) {
        AddressOutputRange range{chain.getAccess()};
        auto grouped = range | ranges::view::group_by([](auto pair1, auto pair2) { return pair1.first == pair2.first; });
        std::map<uint64_t, Address> topAddresses;
        
        RANGES_FOR(auto outputGroup, grouped) {
            auto address = ranges::front(outputGroup).first;
            auto balancesIfUnspent = outputGroup | ranges::view::transform([&](auto pair) -> uint64_t {
                Output out{pair.second, chain.getAccess()};
                return out.isSpent() ? 0 : out.getValue();
            });
            
            uint64_t balance = ranges::accumulate(balancesIfUnspent, uint64_t{0});
            if (topAddresses.size() < 100) {
                topAddresses.insert(std::make_pair(balance, address));
            } else {
                auto lowestVal = topAddresses.begin();
                if (balance > lowestVal->first) {
                    topAddresses.erase(lowestVal);
                    topAddresses.insert(std::make_pair(balance, address));
                }
            }
        }
        return topAddresses;
    }
} // namespace blocksci
