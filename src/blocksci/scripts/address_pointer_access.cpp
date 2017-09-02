//
//  address_pointer_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "address_pointer.hpp"
#include "data_access.hpp"
#include "chain/transaction.hpp"
#include "address.hpp"

namespace blocksci {
    
    std::unique_ptr<Address> AddressPointer::getAddress() const {
        return getAddress(DataAccess::Instance().scripts);
    }
    
    uint32_t AddressPointer::getFirstTransactionIndex() const {
        return getFirstTransactionIndex(DataAccess::Instance().scriptsFirstSeen);
    }
    
    Transaction AddressPointer::getFirstTransaction() const {
        auto &instance = DataAccess::Instance();
        return getFirstTransaction(instance.chain, instance.scriptsFirstSeen);
    }
    
    boost::optional<AddressPointer> getAddressPointerFromString(const std::string &addressString) {
        auto &instance = DataAccess::Instance();
        return getAddressPointerFromString(instance.config, instance.scripts, addressString);
    }
    
    std::vector<AddressPointer> getAddressPointersFromStrings(const std::vector<std::string> &addressStrings) {
        auto &instance = DataAccess::Instance();
        return getAddressPointersFromStrings(instance.config, instance.scripts, addressStrings);
    }
    
    std::vector<AddressPointer> getAddressPointersStartingWithPrefex(const std::string &prefix) {
        auto &instance = DataAccess::Instance();
        return getAddressPointersStartingWithPrefex(instance.config, instance.scripts, prefix);
    }
    
    size_t addressCount() {
        return addressCount(DataAccess::Instance().scripts);
    }
}
