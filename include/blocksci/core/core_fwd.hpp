//
//  core_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/22/18.
//
//

#ifndef blocksci_core_fwd_hpp
#define blocksci_core_fwd_hpp

#include "address_types.hpp"
#include "dedup_address_type.hpp"

namespace blocksci {
    struct RawBlock;
    struct RawTransaction;
    struct Inout;

    struct ScriptDataBase;
    struct PubkeyData;
    struct MultisigData;
    struct ScriptHashData;
    struct RawData;
    struct NonstandardScriptData;
    struct NonstandardSpendScriptData;

    template <DedupAddressType::Enum>
    struct ScriptInfo;

    struct RawAddress;
    
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <DedupAddressType::Enum>
    struct DedupAddressInfo;
    
    template <EquivAddressType::Enum>
    struct EquivAddressInfo;

    class uint256;
    class uint160;
} // namespace blocksci

#endif /* blocksci_core_fwd_hpp */
