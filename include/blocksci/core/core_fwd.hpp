//
//  core_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/22/18.
//
//

#ifndef blocksci_core_fwd_hpp
#define blocksci_core_fwd_hpp

namespace blocksci {
    struct RawBlock;
    struct RawTransaction;
    struct Inout;
    struct InoutPointer;
    
    struct ScriptDataBase;
    struct PubkeyData;
    struct MultisigData;
    struct ScriptHashData;
    struct RawData;
    struct NonstandardScriptData;
    struct NonstandardSpendScriptData;
    struct WitnessUnknownScriptData;
    struct WitnessUnknownSpendScriptData;

    struct RawAddress;
    struct DedupAddress;

    class uint256;
    class uint160;
} // namespace blocksci

#endif /* blocksci_core_fwd_hpp */
