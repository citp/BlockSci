//
//  nonstandard_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "nonstandard_script.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace script;
 
    Nonstandard::ScriptAddress(uint32_t addressNum) : Nonstandard(*DataAccess::Instance().scripts, addressNum) {}
}

