//
//  scriptsfwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scriptsfwd_h
#define scriptsfwd_h

#include <blocksci/address/address_types.hpp>

namespace blocksci {
    struct PubkeyData;
    struct PubkeyHashData;
    struct MultisigData;
    struct ScriptHashData;
    struct RawData;
    struct NonstandardScriptData;
    class ScriptAccess;
    struct DataConfiguration;
    
    template <AddressType::Enum AddressType>
    class ScriptAddress;
    
    namespace script {
        using Pubkey = ScriptAddress<AddressType::Enum::PUBKEY>;
        using PubkeyHash = ScriptAddress<AddressType::Enum::PUBKEYHASH>;
        using ScriptHash = ScriptAddress<AddressType::Enum::SCRIPTHASH>;
        using Multisig = ScriptAddress<AddressType::Enum::MULTISIG>;
        using OpReturn = ScriptAddress<AddressType::Enum::NULL_DATA>;
        using Nonstandard = ScriptAddress<AddressType::Enum::NONSTANDARD>;
    }
}

#endif /* scriptsfwd_h */
