//
//  script_type.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_type.hpp"

namespace blocksci {
    constexpr std::array<ScriptType::Enum, 5> ScriptType::all;
}

namespace std {
    size_t hash<blocksci::ScriptType::Enum>:: operator()(blocksci::ScriptType::Enum val) const {
        return static_cast<size_t>(val);
    };
}
