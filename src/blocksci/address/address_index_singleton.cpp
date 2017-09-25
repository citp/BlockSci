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
    
    std::vector<const Output *> AddressIndex::getOutputs(const Address &address) const {
        return getOutputs(address, *DataAccess::Instance().chain);
    }
    
    std::vector<const Input *> AddressIndex::getInputs(const Address &address) const {
        return getInputs(address, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getTransactions(const Address &address) const {
        return getTransactions(address, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Address &address) const {
        return getOutputTransactions(address, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Address &address) const {
        return getInputTransactions(address, *DataAccess::Instance().chain);
    }
    
    std::vector<const Output *> AddressIndex::getOutputs(const Script &script) const {
        return getOutputs(script, *DataAccess::Instance().chain);
    }
    
    std::vector<const Input *> AddressIndex::getInputs(const Script &script) const {
        return getInputs(script, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getTransactions(const Script &script) const {
        return getTransactions(script, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Script &script) const {
        return getOutputTransactions(script, *DataAccess::Instance().chain);
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Script &script) const {
        return getInputTransactions(script, *DataAccess::Instance().chain);
    }
}
