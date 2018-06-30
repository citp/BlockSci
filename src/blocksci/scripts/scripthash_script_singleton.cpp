//
//  scripthash_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "scripthash_script.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace script;
    ScriptHash::ScriptAddress(uint32_t addressNum) : ScriptHash(DataAccess::Instance().scripts, addressNum) {}
    std::string ScriptHash::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
    
    std::unique_ptr<Script> ScriptHash::wrappedScript() const {
        return wrappedScript(DataAccess::Instance().scripts);
    }
}
