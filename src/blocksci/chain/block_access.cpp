//
//  block_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "block.hpp"
#include "data_access.hpp"
#include "transaction.hpp"
#include "output.hpp"

#include <range/v3/iterator_range.hpp>

namespace blocksci {
    
    Block::Block(uint32_t blockNum) : Block(blockNum, *DataAccess::Instance().chain) {}
    
    bool isSegwit(const Block &block) {
        return isSegwit(block, *DataAccess::Instance().scripts);
    }
    
    std::vector<const Output *> getUnspentOutputs(const Block &block) {
        return getUnspentOutputs(block, *DataAccess::Instance().chain);
    }
    
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height) {
        return totalOutAfterHeight(block, height, *DataAccess::Instance().chain);
    }
    
    std::vector<const Output *> getOutputsSpentByHeight(const Block &block, uint32_t height) {
        return getOutputsSpentByHeight(block, height, *DataAccess::Instance().chain);
    }
    
    uint64_t getTotalSpentOfAge(const Block &block, uint32_t age) {
        return getTotalSpentOfAge(block, *DataAccess::Instance().chain, age);
    }
    
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, uint32_t maxAge) {
        return getTotalSpentOfAges(block, *DataAccess::Instance().chain, maxAge);
    }
    
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block) {
        return netFullTypeValue(block, *DataAccess::Instance().scripts);
    }
}
