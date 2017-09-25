//
//  state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#include "state.hpp"
#include "chain/chain_access.hpp"
#include "chain/block.hpp"
#include "scripts/script_access.hpp"
#include "util.hpp"

namespace blocksci {
    
    State::State(ChainAccess &chain, ScriptAccess &scripts) {
        txCount = chain.txCount();
        blockCount = chain.getBlocks().size();
        scriptCounts = scripts.scriptCounts();
    }
    
    State::State() : blockCount(0) {
        scriptCounts.fill(0);
    }
    
    std::ostream& operator<<(std::ostream& s, const State &data) {
        s << std::hex << data.blockCount << " " << data.txCount << " ";
        for (auto count : data.scriptCounts) {
            s << count << " ";
        }
        s << std::dec;
        return s;
    }
    
    // The read should be able to read the version printed using <<
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
