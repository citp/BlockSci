//
//  blockchain.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/blockchain.hpp>

#include <range/v3/action/push_back.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>

namespace blocksci {
    
    std::vector<BlockRange> BlockRange::segment(unsigned int segmentCount) const {
        std::vector<BlockRange> segments;
        
        if (size() < static_cast<BlockHeight>(segmentCount)) {
            segments.push_back(*this);
            return segments;
        }
        
        auto lastTx = endTxIndex();
        auto firstTx = firstTxIndex();
        auto totalTxCount = lastTx - firstTx;
        uint32_t segmentSize = totalTxCount / segmentCount;
        
        
        auto it = begin();
        auto chainEnd = end();
        while(lastTx - (*it).firstTxIndex() > segmentSize) {
            auto endIt = std::lower_bound(it, chainEnd, (*it).firstTxIndex() + segmentSize, [](const Block &block, uint32_t txNum) {
                return block.firstTxIndex() < txNum;
            });
            auto startBlock = *it;
            auto endBlock = *endIt;
            segments.emplace_back(Slice{startBlock.height(), endBlock.height()}, access);
            it = endIt;
        }
        if (segments.size() == segmentCount) {
            segments.back() = {Slice{segments.back().sl.start, sl.stop}, access};
        } else {
            auto startBlock = *it;
            segments.emplace_back(Slice{startBlock.height(), sl.stop}, access);
        }
        return segments;
    }
    
    std::vector<Block> BlockRange::filter(std::function<bool(const Block &block)> testFunc)  {
        auto mapFunc = [&testFunc](const BlockRange &segment) -> std::vector<Block> {
            return segment | ranges::view::filter(testFunc) | ranges::to_vector;
        };
        
        auto reduceFunc = [] (std::vector<Block> &vec1, std::vector<Block> &vec2) -> std::vector<Block> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        return mapReduce<std::vector<Block>>(mapFunc, reduceFunc);
    }
    
    std::vector<Transaction> BlockRange::filter(std::function<bool(const Transaction &tx)> testFunc)  {
        auto mapFunc = [&testFunc](const BlockRange &segment) -> std::vector<Transaction> {
            std::vector<Transaction> txes;
            for (auto block : segment) {
                txes |= ranges::action::push_back(block | ranges::view::filter(testFunc));
            }
            return txes;
        };
        
        auto reduceFunc = [] (std::vector<Transaction> &vec1, std::vector<Transaction> &vec2) -> std::vector<Transaction> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        return mapReduce<std::vector<Transaction>>(mapFunc, reduceFunc);
    }
} // namespace blocksci
