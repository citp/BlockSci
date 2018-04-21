//
//  nonstandard_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nonstandard_script_hpp
#define nonstandard_script_hpp

#include <blocksci/blocksci_export.h>
#include "script.hpp"
#include "script_view.hpp"

#include <blocksci/util/data_access.hpp>
#include <range/v3/utility/optional.hpp>

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::NONSTANDARD> : public ScriptBase {
        const NonstandardSpendScriptData *rawInputData;
        
        ScriptAddress(uint32_t scriptNum_, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData_, DataAccess &access_) : ScriptBase(scriptNum_, addressType, access_, std::get<0>(rawData_)), rawInputData(std::get<1>(rawData_)) {}
        
        const NonstandardScriptData *getData() const {
            return reinterpret_cast<const NonstandardScriptData *>(ScriptBase::getData());
        }
        
        ranges::optional<CScriptView> getInputScript() const {
            if (rawInputData != nullptr) {
                return CScriptView{rawInputData->scriptData.begin(), rawInputData->scriptData.end()};
            } else {
                return ranges::nullopt;
            }
        }
        
        CScriptView getOutputScript() const {
            return {getData()->scriptData.begin(), getData()->scriptData.end()};
        }
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::NONSTANDARD;
        
        ScriptAddress() = default;
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_), access_) {}
        
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
