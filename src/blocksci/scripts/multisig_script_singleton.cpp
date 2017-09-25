//
//  multisig_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "multisig_script.hpp"
#include "data_access.hpp"
#include "address/address.hpp"

#include <vector>

namespace blocksci {
    using namespace script;
    
    Multisig::ScriptAddress(uint32_t addressNum) : Multisig(*DataAccess::Instance().scripts, addressNum) {}
}
