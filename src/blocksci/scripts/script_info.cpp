//
//  script_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_info.hpp"

#include <sstream>

namespace blocksci {
    constexpr char ScriptInfo<ScriptType::Enum::PUBKEY>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::PUBKEYHASH>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::SCRIPTHASH>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::MULTISIG>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::NULL_DATA>::typeName[];
    constexpr char ScriptInfo<ScriptType::Enum::NONSTANDARD>::typeName[];
    
    template<ScriptType::Enum type>
    struct TypeNameFunctor {
        static std::string f() {
            return ScriptInfo<type>::typeName;
        }
    };
    
    std::string GetTxnOutputType(ScriptType::Enum type) {
        static auto table = make_static_table<TypeNameFunctor>();
        static constexpr std::size_t size = ScriptType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
}
