//
//  script_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef script_info_hpp
#define script_info_hpp

#include "scripts_fwd.hpp"
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/equiv_address_type.hpp>
#include <blocksci/util/util.hpp>

#include <string>

namespace blocksci {
    
    template <typename T>
    struct FixedSize;
    
    template <typename ...T>
    struct Indexed;
    
    template <>
    struct ScriptInfo<EquivAddressType::PUBKEY> {
        using outputType = PubkeyData;
        using storage = FixedSize<PubkeyData>;
    };
    
    template <>
    struct ScriptInfo<EquivAddressType::SCRIPTHASH> {
        using outputType = ScriptHashData;
        using storage = FixedSize<ScriptHashData>;
    };
    
    template <>
    struct ScriptInfo<EquivAddressType::MULTISIG> {
        using outputType = MultisigData;
        using storage = Indexed<MultisigData>;
    };
    
    template <>
    struct ScriptInfo<EquivAddressType::NONSTANDARD> {
        using outputType = NonstandardScriptData;
        using storage = Indexed<NonstandardScriptData,NonstandardSpendScriptData>;
    };
    
    template <>
    struct ScriptInfo<EquivAddressType::NULL_DATA> {
        using outputType = RawData;
        using storage = Indexed<RawData>;
    };
}

#endif /* script_info_hpp */
