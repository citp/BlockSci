//
//  block_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "block.hpp"
#include "util/data_access.hpp"
#include "output.hpp"

#include <range/v3/iterator_range.hpp>

namespace blocksci {
    
    Block::Block(BlockHeight blockNum) : Block(blockNum, *DataAccess::Instance().chain) {}
    
    bool isSegwit(const Block &block) {
        return isSegwit(block, *DataAccess::Instance().scripts);
    }
    
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, BlockHeight maxAge) {
        return getTotalSpentOfAges(block, *DataAccess::Instance().chain, maxAge);
    }
    
    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block) {
        return netFullTypeValue(block, *DataAccess::Instance().scripts);
    }
}
