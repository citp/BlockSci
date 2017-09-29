//
//  script_pointer_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/29/17.
//


#include "script_pointer.hpp"
#include "data_access.hpp"
#include "script.hpp"

namespace blocksci {
    std::unique_ptr<Script> ScriptPointer::getScript() const {
        return getScript(*DataAccess::Instance().scripts);
    }
}
