//
//  script_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef script_info_hpp
#define script_info_hpp

#include "script_type.hpp"
#include "scripts_fwd.hpp"
#include <blocksci/address/address_types.hpp>
#include <blocksci/util/util.hpp>

#include <string>

namespace blocksci {
    
    template <typename T>
    struct FixedSize;
    
    template <typename ...T>
    struct Indexed;
    
    template <>
    struct ScriptInfo<ScriptType::Enum::PUBKEY> {
        static constexpr char name[] = "pubkey_script";
        static constexpr bool deduped = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 3> addressTypes = {{AddressType::PUBKEY, AddressType::PUBKEYHASH, AddressType::WITNESS_PUBKEYHASH}};
        using outputType = PubkeyData;
        using storage = FixedSize<PubkeyData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::SCRIPTHASH> {
        static constexpr char name[] = "scripthash_script";
        static constexpr bool deduped = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = false;
        static constexpr std::array<AddressType::Enum, 2> addressTypes = {{AddressType::SCRIPTHASH, AddressType::WITNESS_SCRIPTHASH}};
        using outputType = ScriptHashData;
        using storage = FixedSize<ScriptHashData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::MULTISIG> {
        static constexpr char name[] = "multisig_script";
        static constexpr bool deduped = true;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::MULTISIG}};
        using outputType = MultisigData;
        using storage = Indexed<MultisigData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::NONSTANDARD> {
        static constexpr char name[] = "nonstandard_script";
        static constexpr bool deduped = false;
        static constexpr bool spendable = true;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NONSTANDARD}};
        using outputType = NonstandardScriptData;
        using storage = Indexed<NonstandardScriptData,NonstandardSpendScriptData>;
    };
    
    template <>
    struct ScriptInfo<ScriptType::Enum::NULL_DATA> {
        static constexpr char name[] = "null_data_script";
        static constexpr bool deduped = false;
        static constexpr bool spendable = false;
        static constexpr bool indexed = true;
        static constexpr std::array<AddressType::Enum, 1> addressTypes = {{AddressType::NULL_DATA}};
        using outputType = RawData;
        using storage = Indexed<RawData>;
    };
    using ScriptInfoList = array_to_tuple_t<ScriptType::Enum, ScriptType::size, ScriptType::all>;
    
    template <template<ScriptType::Enum> class K>
    using to_script_tuple_t = apply_template_t<ScriptType::Enum, K, ScriptInfoList>;
    
    template<ScriptType::Enum type>
    struct SpendableFunctor {
        static constexpr bool f() {
            return ScriptInfo<type>::spendable;
        }
    };

    constexpr void scriptTypeCheckThrow(size_t index) {  
        index >= ScriptType::size ? throw std::invalid_argument("combination of enum values is not valid") : 0;
    }
    
    static constexpr auto spendableTable = blocksci::make_static_table<ScriptType, SpendableFunctor>();
    
    constexpr bool isSpendable(ScriptType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return spendableTable[index];
    }
    
    template<ScriptType::Enum type>
    struct DedupedFunctor {
        static constexpr bool f() {
            return ScriptInfo<type>::deduped;
        }
    };
    
    static constexpr auto dedupedTable = blocksci::make_static_table<ScriptType, DedupedFunctor>();
    
    constexpr bool isDeduped(ScriptType::Enum t) {
        auto index = static_cast<size_t>(t);
        scriptTypeCheckThrow(index);
        return dedupedTable[index];
    }
    
    std::string scriptName(ScriptType::Enum type);
}

#endif /* script_info_hpp */
