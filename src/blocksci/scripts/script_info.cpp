//
//  script_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#include "script_info.hpp"
#include <string>


namespace blocksci {
    constexpr char ScriptInfo<ScriptType::Enum::PUBKEY>::name[];
    constexpr char ScriptInfo<ScriptType::Enum::SCRIPTHASH>::name[];
    constexpr char ScriptInfo<ScriptType::Enum::MULTISIG>::name[];
    constexpr char ScriptInfo<ScriptType::Enum::NULL_DATA>::name[];
    constexpr char ScriptInfo<ScriptType::Enum::NONSTANDARD>::name[];
    
    template<ScriptType::Enum type>
    struct ScriptNameFunctor {
        static std::string f() {
            return ScriptInfo<type>::name;
        }
    };
    
    std::string scriptName(ScriptType::Enum type) {
        static auto table = make_static_table<ScriptType, ScriptNameFunctor>();
        auto index = static_cast<size_t>(type);
        if (index >= ScriptType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index];
    }
}
