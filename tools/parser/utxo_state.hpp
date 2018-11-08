//
//  utxo_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef utxo_state_hpp
#define utxo_state_hpp

#include "serializable_map.hpp"
#include "basic_types.hpp"
#include "utxo.hpp"

#include <blocksci/core/inout_pointer.hpp>

// Map of the current UTXO set of the parser
class UTXOState : public SerializableMap<RawOutputPointer, UTXO> {
public:
    UTXOState() : SerializableMap<RawOutputPointer, UTXO>({blocksci::uint256{}, 0}, {blocksci::uint256{}, 1}) {}
};

class UTXOScriptState : public SerializableMap<blocksci::InoutPointer, uint32_t> {
public:
    UTXOScriptState() : SerializableMap<blocksci::InoutPointer, uint32_t>({std::numeric_limits<uint32_t>::max(), 0}, {std::numeric_limits<uint32_t>::max(), 1}) {}
};


#endif /* utxo_state_hpp */
