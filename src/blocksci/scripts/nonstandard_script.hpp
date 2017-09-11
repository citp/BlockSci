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
#include "bitcoin_script.hpp"

namespace blocksci {
    template <>
    class ScriptAddress<ScriptType::Enum::NONSTANDARD> : public Script {
        boost::optional<CScript> inputScript;
        CScript outputScript;
        
    public:
        ScriptAddress<ScriptType::Enum::NONSTANDARD>(std::tuple<const NonstandardScriptData *, const NonstandardScriptData *> &&rawData);
        ScriptAddress<ScriptType::Enum::NONSTANDARD>(const ScriptAccess &access, uint32_t addressNum);
        
        std::string inputString() const;
        std::string outputString() const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Script &other) override;
        
#ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<AddressType::Enum::NONSTANDARD>(uint32_t addressNum);
#endif
    };
}

#endif /* nonstandard_script_hpp */
