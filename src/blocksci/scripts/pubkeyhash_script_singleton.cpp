//
//  pubkeyhash_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "pubkeyhash_script.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace script;
    
    PubkeyHash::ScriptAddress(uint32_t addressNum) : PubkeyHash(DataAccess::Instance().scripts, addressNum) {}
    std::string PubkeyHash::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
}
