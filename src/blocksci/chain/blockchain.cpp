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

#include "address/address_index.hpp"
#include "chain/chain_access.hpp"

#include "data_configuration.hpp"
#include "data_access.hpp"

#include <fstream>
#include <iostream>

namespace blocksci {
    // [start, end)
    std::vector<std::vector<Block>> segmentChain(const Blockchain &chain, size_t startBlock, size_t endBlock, unsigned int segmentCount) {
        auto &firstBlock = *(chain.begin() + startBlock);
        auto lastBlockIt = chain.begin();
        std::advance(lastBlockIt, static_cast<int64_t>(endBlock));
        auto lastTx = (lastBlockIt - 1)->firstTxIndex + (lastBlockIt - 1)->numTxes;
        auto firstTx = firstBlock.firstTxIndex;
        auto totalTxCount = lastTx - firstTx;
        double segmentSize = static_cast<double>(totalTxCount) / segmentCount;
        
        std::vector<uint32_t> txIndexes;
        txIndexes.reserve(chain.size());
        for (uint32_t i = 0; i < chain.size(); i++) {
            txIndexes.push_back(chain[i].firstTxIndex);
        }
        
        std::vector<std::vector<Block>> segments;
        
        auto it = txIndexes.begin();
        std::advance(it, static_cast<int64_t>(startBlock));
        while(lastTx - *it > segmentSize) {
            auto segmentStart = std::distance(txIndexes.begin(), it);
            auto breakPoint = *it + segmentSize;
            auto endIt = txIndexes.begin();
            std::advance(endIt, static_cast<int64_t>(endBlock));
            it = std::lower_bound(it, endIt, breakPoint);
            auto segmentEnd = std::distance(txIndexes.begin(), it);
            segments.push_back(std::vector<Block>(chain.begin() + segmentStart, chain.begin() + segmentEnd));
        }
        auto segmentStart = std::distance(txIndexes.begin(), it);
        if (segments.size() == segmentCount) {
            segments.back().insert(segments.back().end(), chain.begin() + segmentStart, lastBlockIt);
        } else {
            segments.push_back(std::vector<Block>(chain.begin() + segmentStart, lastBlockIt));
        }
        
        decltype(totalTxCount) totalCount = 0;
        
        std::vector<size_t> segmentSizes;
        for (auto &segment : segments) {
            uint32_t count = 0;
            for (auto &block : segment) {
                count += block.numTxes;
            }
            segmentSizes.push_back(count);
            totalCount += count;
        }
        
        assert(totalCount == totalTxCount);
        
        
        
        return segments;
    }
    
    Blockchain::Blockchain(const std::string &dataDirectory) : Blockchain(DataConfiguration{dataDirectory}, true, 0) {}
    
    Blockchain::Blockchain(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored) : access(DataAccess::Instance(config, errorOnReorg, blocksIgnored)) {}
    
    std::vector<Block>::size_type Blockchain::size() const {
        return access.chain->getBlocks().size();
    }
    boost::iterator_range<const Block *>::const_iterator Blockchain::begin() const {
        return static_cast<const Block *>(access.chain->getBlocks().begin());
    }
    boost::iterator_range<const Block *>::const_iterator Blockchain::end() const {
        return static_cast<const Block *>(access.chain->getBlocks().end());
    }
    const Block& Blockchain::operator[] (const uint32_t index) const {
        return static_cast<const Block &>(access.chain->getBlocks()[index]);
    }
    
    uint32_t txCount(const Blockchain &chain) {
        auto lastBlock = chain.end();
        lastBlock--;
        return lastBlock->firstTxIndex + lastBlock->numTxes;
    }
    
    Transaction Blockchain::txAtIndex(uint32_t index) const {
        return Transaction::txWithIndex(*access.chain, index);
    }
    
    TransactionIterator Blockchain::beginTransactions(uint32_t blockNum) {
        auto &block = this->operator[](blockNum);
        return TransactionIterator(access.chain.get(), block.firstTxIndex, blockNum);
    }
    
    TransactionIterator Blockchain::endTransactions(uint32_t blockNum) {
        auto &block = this->operator[](blockNum - 1);
        return TransactionIterator(access.chain.get(), block.firstTxIndex + block.numTxes, blockNum);
    }
    
    std::vector<Transaction> getCoinjoinTransactions(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock)  {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return isCoinjoin(tx);
        });
    }
    
    std::pair<std::vector<Transaction>, std::vector<Transaction>> getPossibleCoinjoinTransactions(const Blockchain &chain, uint64_t minBaseFee, double percentageFee, size_t maxDepth)  {
        
        auto mapFunc = [&](const std::vector<Block> &segment) {
            std::vector<Transaction> skipped;
            std::vector<Transaction> txes;
            for (auto &block : segment) {
                for (auto tx : block.txes(*chain.access.chain)) {
                    auto label = isPossibleCoinjoin(tx, minBaseFee, percentageFee, maxDepth);
                    if (label == CoinJoinResult::True) {
                        txes.push_back(tx);
                    } else if (label == CoinJoinResult::Timeout) {
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
        
        std::pair<std::vector<Transaction>, std::vector<Transaction>> result;
        chain.mapReduce<RetType, RetType>(0, chain.size(), mapFunc, reduceFunc, result);
        return result;
    }
    
    std::vector<Block> filter(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, std::function<bool(const Block &tx)> testFunc)  {
        auto mapFunc = [&chain, &testFunc](const std::vector<Block> &segment) -> std::vector<Block> {
            std::vector<Block> blocks;
            for (auto &block : segment) {
                if (testFunc(block)) {
                    blocks.push_back(block);
                }
            }
            return blocks;
        };
        
        auto reduceFunc = [] (std::vector<Block> &vec1, std::vector<Block> &vec2) -> std::vector<Block> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        std::vector<Block> blocks;
        return chain.mapReduce<std::vector<Block>, std::vector<Block>>(startBlock, endBlock, mapFunc, reduceFunc, blocks);
    }
    
    std::vector<Transaction> filter(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, std::function<bool(const Transaction &tx)> testFunc)  {
        auto mapFunc = [&chain, &testFunc](const std::vector<Block> &segment) -> std::vector<Transaction> {
            std::vector<Transaction> txes;
            for (auto &block : segment) {
                for (auto tx : block.txes(*chain.access.chain)) {
                    if (testFunc(tx)) {
                        txes.push_back(tx);
                    }
                }
            }
            return txes;
        };
        
        auto reduceFunc = [] (std::vector<Transaction> &vec1, std::vector<Transaction> &vec2) -> std::vector<Transaction> & {
            vec1.reserve(vec1.size() + vec2.size());
            vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
            return vec1;
        };
        
        std::vector<Transaction> txes;
        return chain.mapReduce<std::vector<Transaction>, std::vector<Transaction>>(startBlock, endBlock, mapFunc, reduceFunc, txes);
    }
    
    std::vector<Transaction> getTransactionIncludingOutput(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, AddressType::Enum type) {
        return filter(chain, startBlock, endBlock, [type](const Transaction &tx) {
            for (auto &output : tx.outputs()) {
                if (output.getType() == type) {
                    return true;
                }
            }
            return false;
        });
    }
    
    std::vector<Transaction> getDeanonTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return isDeanonTx(tx);
        });
    }
    
    std::vector<Transaction> getChangeOverTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return isChangeOverTx(tx);
        });
    }
    
    std::vector<Transaction> getKeysetChangeTxes(const Blockchain &chain, uint32_t startBlock, uint32_t endBlock) {
        return filter(chain, startBlock, endBlock, [](const Transaction &tx) {
            return containsKeysetChange(tx);
        });
    }
}
