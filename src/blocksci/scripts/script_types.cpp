//
//  script_types.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 2/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_types.hpp"
#include "script_info.hpp"
#include <iostream>
#include <stdio.h>


namespace blocksci {
    
}

std::ostream &operator<<(std::ostream &os, blocksci::ScriptType::Enum const &type) {
    os << GetTxnOutputType(type);
    return os;
}
