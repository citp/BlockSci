//
//  nonstandard_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nonstandard_script_hpp
#define nonstandard_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include "script_view.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    template <>
    class ScriptAddress<ScriptType::Enum::NONSTANDARD> : public BaseScript {
        ranges::optional<CScriptView> inputScript;
        CScriptView outputScript;
        
    public:
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::NONSTANDARD;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string inputString() const;
        std::string outputString() const;
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;
        
#ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<scriptType>(uint32_t addressNum);
        std::string toString() const;
        std::string toPrettyString() const;
#endif
    };
}

#endif /* nonstandard_script_hpp */
