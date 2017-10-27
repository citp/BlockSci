//
//  nulldata_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nulldata_script_hpp
#define nulldata_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include "bitcoin_script.hpp"

namespace blocksci {
    struct RawData;
    
    template <>
    class ScriptAddress<ScriptType::Enum::NULL_DATA> : public Script {
        
    public:
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::NULL_DATA;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const RawData *rawData);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        std::string data;
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;

        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<scriptType>(uint32_t addressNum);
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
    };
}

#endif /* nulldata_script_hpp */
