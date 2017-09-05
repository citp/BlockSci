//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include <stdio.h>

#include "script.hpp"
#include "data_access.hpp"

namespace blocksci {
    std::unique_ptr<Script> Script::create(const Address &pointer) {
        return create(DataAccess::Instance().scripts, pointer);
    }
    
    std::string Script::toPrettyString() const {
        auto &instance = DataAccess::Instance();
        return toPrettyString(instance.config, instance.scripts);
    }
    
    std::string Script::toString() const {
        return toString(DataAccess::Instance().config);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Script &script) {
    os << script.toString();
    return os;
}
