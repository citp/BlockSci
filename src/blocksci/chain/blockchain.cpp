//
//  blockchain.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "blockchain.hpp"
#include "block.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "heuristics/tx_identification.hpp"

#include "chain/chain_access.hpp"

#include "util/data_configuration.hpp"

#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/algorithm/any_of.hpp>

#include <fstream>
#include <iostream>

namespace blocksci {
    // [start, end)
    std::vector<std::vector<Block>> segmentChain(const Blockchain &chain, int startBlock, int endBlock, unsigned int segmentCount) {
        auto lastTx = chain[endBlock - 1].endTxIndex();
        auto firstTx = chain[startBlock].firstTxIndex();
        auto totalTxCount = lastTx - firstTx;
        double segmentSize = static_cast<double>(totalTxCount) / segmentCount;
        
        std::vector<std::vector<Block>> segments;
        auto it = chain.begin();
        std::advance(it, startBlock);
        auto chainEnd = chain.begin();
        std::advance(chainEnd, endBlock);
        while(lastTx - (*it).firstTxIndex() > segmentSize) {
            auto endIt = std::lower_bound(it, chainEnd, (*it).firstTxIndex() + segmentSize, [](const Block &block, uint32_t txNum) {
                return block.firstTxIndex() < txNum;
            });
            segments.push_back(std::vector<Block>(it, endIt));
            it = endIt;
        }
        if (segments.size() == segmentCount) {
            segments.back().insert(segments.back().end(), it, chainEnd);
        } else {
            segments.push_back(std::vector<Block>(it, chainEnd));
        }
        return segments;
    }
    
    Blockchain::Blockchain(const std::string &dataDirectory) : Blockchain(DataConfiguration{dataDirectory}, true, 0) {}
    
    Blockchain::Blockchain(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored) : access(&DataAccess::Instance(config, errorOnReorg, blocksIgnored)) {
        lastBlockHeight = access->chain->blockCount();
    }
    
    template<ScriptType::Enum type>
    struct ScriptRangeFunctor {
        static ScriptRangeVariant f(const Blockchain &chain) {
            return chain.scripts<type>();
        }
    };
    
    ScriptRangeVariant Blockchain::scripts(ScriptType::Enum type) const {
        static auto table = make_static_table<ScriptType, ScriptRangeFunctor>(*this);
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
    
    uint32_t txCount(const Blockchain &chain) {
        auto lastBlock = chain[chain.size() - 1];
        return lastBlock.endTxIndex();
    }
    
    std::vector<Block> filter(const Blockchain &chain, int startBlock, int endBlock, std::function<bool(const Block &tx)> testFunc)  {
        auto mapFunc = [&chain, &testFunc](const std::vector<Block> &segment) -> std::vector<Block> {
            return segment | ranges::view::filter(testFunc) | ranges::to_vector;
        };
        
        auto reduceFunc = [] (std::vector<Block> &vec1, std::vector<Block> &vec2) -> std::vector<Block> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        return chain.mapReduce<std::vector<Block>>(startBlock, endBlock, mapFunc, reduceFunc);
    }
    
    std::vector<Transaction> filter(const Blockchain &chain, int startBlock, int endBlock, std::function<bool(const Transaction &tx)> testFunc)  {
        auto mapFunc = [&chain, &testFunc](const std::vector<Block> &segment) -> std::vector<Transaction> {
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
    
    std::vector<Transaction> getTransactionIncludingOutput(const Blockchain &chain, int startBlock, int endBlock, AddressType::Enum type) {
        return filter(chain, startBlock, endBlock, [type](const Transaction &tx) {
            return ranges::any_of(tx.outputs(), [=](const Output &output) {
                return output.getType() == type;
            });
        });
    }
}
