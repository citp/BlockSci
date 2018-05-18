//
//  state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#ifndef state_hpp
#define state_hpp

#include "dedup_address_type.hpp"

#include <array>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    
    struct State {
        uint32_t blockCount;
        uint32_t txCount;
        std::array<uint32_t, DedupAddressType::size> scriptCounts;
        
        State(const ChainAccess &chain, const ScriptAccess &scripts);
        State() : blockCount(0), txCount(0), scriptCounts{} {
            scriptCounts.fill(0);
        }
    };
    
    std::ostream& operator<<(std::ostream& s, const State &data);
    std::istream& operator>>(std::istream& s, State &data);
}

#endif /* state_hpp */
