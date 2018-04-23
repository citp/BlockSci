//
//  state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/24/17.
//

#ifndef state_hpp
#define state_hpp

#include <blocksci/core/chain_access.hpp>
#include <blocksci/core/dedup_address_type.hpp>
#include <blocksci/core/script_access.hpp>

namespace blocksci {
    struct State {
        uint32_t blockCount;
        uint32_t txCount;
        std::array<uint32_t, DedupAddressType::size> scriptCounts;
        
        State(const ChainAccess &chain, const ScriptAccess &scripts) : blockCount{static_cast<uint32_t>(static_cast<int>(chain.blockCount()))}, txCount{static_cast<uint32_t>(chain.txCount())}, scriptCounts{scripts.scriptCounts()} {}
        State() : blockCount(0), txCount(0), scriptCounts{} {
            scriptCounts.fill(0);
        }
    };
    
    inline std::ostream& operator<<(std::ostream& s, const State &data) {
        s << std::hex << data.blockCount << " " << data.txCount << " ";
        for (auto count : data.scriptCounts) {
            s << count << " ";
        }
        s << std::dec;
        return s;
    }
    
    inline std::istream& operator>>(std::istream& s, State &data) {
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

#endif /* state_hpp */
