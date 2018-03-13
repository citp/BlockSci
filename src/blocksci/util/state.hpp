//
//  state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#ifndef state_hpp
#define state_hpp

#include <blocksci/address/equiv_address_type.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>

namespace blocksci {
    struct State {
        uint32_t blockCount;
        uint32_t txCount;
        std::array<uint32_t, EquivAddressType::size> scriptCounts;
        
        State(const ChainAccess &chain, const ScriptAccess &scripts);
        State();
    };
    
    std::ostream& operator<<(std::ostream& s, const State &data);
    std::istream& operator>>(std::istream& s, State &data);
}

#endif /* state_hpp */
