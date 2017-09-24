//
//  update_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#ifndef update_state_hpp
#define update_state_hpp

#include "scripts/script_type.hpp"
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    
    struct UpdateState {
        uint32_t blockCount;
        uint32_t txCount;
        std::array<uint32_t, ScriptType::size> scriptCounts;
        
        UpdateState(ChainAccess &chain, ScriptAccess &scripts);
    };
}

#endif /* update_state_hpp */
