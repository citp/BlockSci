//
//  address_index_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "address_index.hpp"
#include "data_access.hpp"
#include "chain/transaction.hpp"

namespace blocksci {
    
    std::vector<const Output *> AddressIndex::getOutputs(const AddressPointer &address) const {
        return getOutputs(DataAccess::Instance().chain, address);
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const AddressPointer &address) const {
        return getOutputTransactions(DataAccess::Instance().chain, address);
    }
}
