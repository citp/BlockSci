//
//  transaction_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "transaction.hpp"
#include "util/data_access.hpp"
#include "output.hpp"
#include "input.hpp"
#include "block.hpp"
#include "util/bitcoin_uint256.hpp"

namespace blocksci {
    
    Transaction::Transaction(uint256 hash) : Transaction(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain) {}
    
    Transaction::Transaction(std::string hash) : Transaction(hash, *DataAccess::Instance().hashIndex, *DataAccess::Instance().chain) {}
    
    Transaction::Transaction(uint32_t index) : Transaction(index, *DataAccess::Instance().chain) {}
    
    Transaction::Transaction(uint32_t index, uint32_t height) : Transaction(index, height, *DataAccess::Instance().chain) {}
}
