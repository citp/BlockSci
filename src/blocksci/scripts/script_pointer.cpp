//
//  script_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/19/17.
//
//

#include "script_pointer.hpp"
#include "address/address_info.hpp"
#include "address/address.hpp"

namespace blocksci {
    ScriptPointer::ScriptPointer(const Address &address) : ScriptPointer(address.addressNum, scriptType(address.type)) {}
}
