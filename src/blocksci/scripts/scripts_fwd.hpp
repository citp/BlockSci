//
//  scriptsfwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scriptsfwd_h
#define scriptsfwd_h

#include <blocksci/address/dedup_address_type.hpp>
#include <blocksci/address/address_types.hpp>

namespace blocksci {
    template <DedupAddressType::Enum>
    struct ScriptData;
    
    class CBitcoinAddress;
    class CKeyID;
    class CPubKey;
    struct ScriptDataBase;
    struct PubkeyData;
    struct MultisigData;
    struct ScriptHashData;
    struct RawData;
    struct NonstandardScriptData;
    struct NonstandardSpendScriptData;
    class ScriptAccess;
    struct DataConfiguration;
    
    template<typename T>
    class ScriptBase;
    
    class AnyScript;
    class CScriptView;
    
    template <AddressType::Enum>
    class ScriptAddress;
    
    template <DedupAddressType::Enum>
    struct ScriptInfo;
    
    namespace script {
        using Pubkey = ScriptAddress<AddressType::PUBKEY>;
        using PubkeyHash = ScriptAddress<AddressType::PUBKEYHASH>;
        using MultisigPubkey = ScriptAddress<AddressType::MULTISIG_PUBKEY>;
        using WitnessPubkeyHash = ScriptAddress<AddressType::WITNESS_PUBKEYHASH>;
        using ScriptHash = ScriptAddress<AddressType::SCRIPTHASH>;
        using WitnessScriptHash = ScriptAddress<AddressType::WITNESS_SCRIPTHASH>;
        using Multisig = ScriptAddress<AddressType::MULTISIG>;
        using OpReturn = ScriptAddress<AddressType::NULL_DATA>;
        using Nonstandard = ScriptAddress<AddressType::NONSTANDARD>;
    }
}

#endif /* scriptsfwd_h */
