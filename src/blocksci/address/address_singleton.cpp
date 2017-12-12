//
//  address_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "address.hpp"
#include "util/data_access.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"
#include "scripts/script.hpp"
#include "scripts/script_variant.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    
    AnyScript Address::getScript() const {
        return getScript(*DataAccess::Instance().scripts);
    }
    
    uint64_t calculateBalance() const {
        return calculateBalance(*instance.addressIndex, *instance.chain);
    }
    
    uint64_t calculateBalanceAtHeight(uint32_t height) const {
        return calculateBalanceAtHeight(height, *instance.addressIndex, *instance.chain);
    }
        
    std::vector<Output> Address::getOutputs() const {
        auto &instance = DataAccess::Instance();
        return getOutputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Input> Address::getInputs() const {
        auto &instance = DataAccess::Instance();
        return getInputs(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        auto &instance = DataAccess::Instance();
        return getTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getOutputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getOutputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        auto &instance = DataAccess::Instance();
        return getInputTransactions(*instance.addressIndex, *instance.chain);
    }
    
    ranges::optional<Address> getAddressFromString(const std::string &addressString) {
        auto &instance = DataAccess::Instance();
        return getAddressFromString(instance.config, *instance.hashIndex, addressString);
    }
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix) {
        return getAddressesWithPrefix(prefix, *DataAccess::Instance().scripts);
    }
    
    size_t addressCount() {
        return addressCount(*DataAccess::Instance().scripts);
    }

    std::string Address::fullType() const {
        return fullType(*DataAccess::Instance().scripts);
    }
    
}
