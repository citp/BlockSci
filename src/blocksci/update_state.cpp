//
//  update_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#include "update_state.hpp"
#include "chain/chain_access.hpp"
#include "chain/block.hpp"
#include "scripts/script_access.hpp"
#include "util.hpp"

namespace blocksci {
    
    template<AddressType::Enum type>
    struct ScriptCountFunctor {
        static constexpr std::string_view f() {
            return AddressInfo<type>::name;
        }
    };
    
    UpdateState::UpdateState(ChainAccess &chain, ScriptAccess &scripts) {
        txCount = chain.txCount();
        blockCount = chain.getBlocks().size();
        scriptCounts = scripts.scriptCounts();
    }
}
