//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include <stdio.h>

#include "address.hpp"
#include "data_access.hpp"

namespace blocksci {
    std::unique_ptr<Address> Address::create(const AddressPointer &pointer) {
        return create(DataAccess::Instance().scripts, pointer);
    }
    
    std::string Address::toPrettyString() const {
        auto &instance = DataAccess::Instance();
        return toPrettyString(instance.config, instance.scripts);
    }
    
    std::string Address::toString() const {
        return toString(DataAccess::Instance().config);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address) {
    os << address.toString();
    return os;
}
