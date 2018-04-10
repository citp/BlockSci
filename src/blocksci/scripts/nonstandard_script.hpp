//
//  nonstandard_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nonstandard_script_hpp
#define nonstandard_script_hpp

#include "script_view.hpp"
#include "script.hpp"
#include "script_access.hpp"

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    template <>
    class ScriptAddress<AddressType::NONSTANDARD> : public ScriptBase<ScriptAddress<AddressType::NONSTANDARD>> {
        friend class ScriptBase<ScriptAddress<AddressType::NONSTANDARD>>;
        const NonstandardScriptData *rawData;
        const NonstandardSpendScriptData *rawInputData;
        
        ScriptAddress(uint32_t scriptNum_, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData_, DataAccess &access_) : ScriptBase(scriptNum_, addressType, access_), rawData(std::get<0>(rawData_)), rawInputData(std::get<1>(rawData_)) {}
        
        ranges::optional<CScriptView> getInputScript() const {
            if (rawInputData != nullptr) {
                return CScriptView{rawInputData->scriptData.begin(), rawInputData->scriptData.end()};
            } else {
                return ranges::nullopt;
            }
        }
        
        CScriptView getOutputScript() const {
            return {rawData->scriptData.begin(), rawData->scriptData.end()};
        }
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::NONSTANDARD;
        
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.scripts.getScriptData<addressType>(addressNum_), access_) {}
        
        std::string inputString() const {
            auto inputScript = getInputScript();
            if (inputScript) {
                return ScriptToAsmStr(*inputScript);
            } else {
                return "No input script";
            }
        }
        
        std::string outputString() const {
            return ScriptToAsmStr(getOutputScript());
        }
        
        std::string toString() const {
            std::stringstream ss;
            ss << "NonStandardScript()";
            return ss.str();
        }
        
        std::string toPrettyString() const {
            std::stringstream ss;
            ss << "NonStandardScript(" << inputString() << ", " << outputString() << ")";
            return ss.str();
        }
    };
} // namespace blocksci

#endif /* nonstandard_script_hpp */
