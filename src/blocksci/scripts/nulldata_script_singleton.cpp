//
//  nulldata_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "nulldata_script.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace script;
    
    OpReturn::ScriptAddress(uint32_t addressNum) : OpReturn(DataAccess::Instance().scripts, addressNum) {}
}
