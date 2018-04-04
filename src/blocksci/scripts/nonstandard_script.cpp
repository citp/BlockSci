//
//  nonstandard_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "nonstandard_script.hpp"

#include "script_access.hpp"
#include "script_data.hpp"

#include <blocksci/util/data_access.hpp>

namespace blocksci {
    using script::Nonstandard;
    
    Nonstandard::ScriptAddress(uint32_t scriptNum_, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData_, const DataAccess &access_) : ScriptBase(scriptNum_, addressType, access_), rawData(std::get<0>(rawData_)), rawInputData(std::get<1>(rawData_)) {}
    
    Nonstandard::ScriptAddress(uint32_t addressNum_, const DataAccess &access_) : Nonstandard(addressNum_, access_.scripts->getScriptData<addressType>(addressNum_), access_) {}
    
    CScriptView Nonstandard::getOutputScript() const {
        return rawData->getScript();
    }
    
    ranges::optional<CScriptView> Nonstandard::getInputScript() const {
        if (rawInputData != nullptr) {
            return rawInputData->getScript();
        } else {
            return ranges::nullopt;
        }
    }
    
    std::string Nonstandard::inputString() const {
        auto inputScript = getInputScript();
        if (inputScript) {
            return ScriptToAsmStr(*inputScript);
        } else {
            return "No input script";
        }
    }
    
    std::string Nonstandard::outputString() const {
        return ScriptToAsmStr(getOutputScript());
    }
    
    std::string Nonstandard::toString() const {
        std::stringstream ss;
        ss << "NonStandardScript()";
        return ss.str();
    }
    
    std::string Nonstandard::toPrettyString() const {
        std::stringstream ss;
        ss << "NonStandardScript(" << inputString() << ", " << outputString() << ")";
        return ss.str();
    }
} // namespace blocksci
