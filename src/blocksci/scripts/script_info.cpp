//
//  script_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#include "script_info.hpp"


namespace blocksci {
    constexpr char ScriptInfo<ScriptType::Enum::PUBKEY>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::SCRIPTHASH>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::MULTISIG>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::NULL_DATA>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::NONSTANDARD>::typeName[];
}
