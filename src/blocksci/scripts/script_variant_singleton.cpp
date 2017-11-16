//
//  script_variant_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/23/17.
//

#include "script_variant.hpp"
#include "scripts.hpp"
#include "data_access.hpp"

namespace blocksci {
    AnyScript::AnyScript(const Address &address) : AnyScript(address, *DataAccess::Instance().scripts) {}
    AnyScript::AnyScript(const Script &script) : AnyScript(script, *DataAccess::Instance().scripts) {}
}
