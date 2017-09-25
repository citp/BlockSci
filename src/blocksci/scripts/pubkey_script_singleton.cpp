//
//  pubkey_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "pubkey_script.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace script;
    
    Pubkey::ScriptAddress(uint32_t addressNum) : Pubkey(*DataAccess::Instance().scripts, addressNum) {}
    
    std::string Pubkey::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
}
