//
//  state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/24/18.
//
//

#include "state.hpp"
#include "chain_access.hpp"
#include "script_access.hpp"

#include <iostream>

namespace blocksci {
    State::State(const ChainAccess &chain, const ScriptAccess &scripts) : blockCount{static_cast<uint32_t>(static_cast<int>(chain.blockCount()))}, txCount{static_cast<uint32_t>(chain.txCount())}, scriptCounts{scripts.scriptCounts()} {}
    
    std::ostream& operator<<(std::ostream& s, const State &data) {
        s << std::hex << data.blockCount << " " << data.txCount << " ";
        for (auto count : data.scriptCounts) {
            s << count << " ";
        }
        s << std::dec;
        return s;
    }
    
    std::istream& operator>>(std::istream& s, State &data) {
        // Something like this
        // Be careful with strings.
        s >> std::hex >> data.blockCount >> data.txCount;
        for (auto &count : data.scriptCounts) {
            s >> count;
        }
        s >> std::dec;
        return s;
    }
}
