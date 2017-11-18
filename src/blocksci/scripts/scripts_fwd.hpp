//
//  scriptsfwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scriptsfwd_h
#define scriptsfwd_h

#include "script_type.hpp"

namespace blocksci {
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
    struct Script;
    class AnyScript;
    class CScriptView;
    
    template <auto>
    class ScriptAddress;
    
    template <ScriptType::Enum>
    struct ScriptInfo;
    
    namespace script {
        using Pubkey = ScriptAddress<ScriptType::Enum::PUBKEY>;
        using ScriptHash = ScriptAddress<ScriptType::Enum::SCRIPTHASH>;
        using Multisig = ScriptAddress<ScriptType::Enum::MULTISIG>;
        using OpReturn = ScriptAddress<ScriptType::Enum::NULL_DATA>;
        using Nonstandard = ScriptAddress<ScriptType::Enum::NONSTANDARD>;
    }
}

#endif /* scriptsfwd_h */
