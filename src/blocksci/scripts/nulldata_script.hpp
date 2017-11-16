//
//  nulldata_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nulldata_script_hpp
#define nulldata_script_hpp

#include "script.hpp"

#include <string>

namespace blocksci {
 
    template <>
    class ScriptAddress<ScriptType::Enum::NULL_DATA> : public BaseScript {
        
    public:
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::NULL_DATA;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const RawData *rawData, const ScriptAccess &access);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        std::string data;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
}

#endif /* nulldata_script_hpp */
