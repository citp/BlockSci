//
//  nonstandard_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "nonstandard_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    using namespace script;
    Nonstandard::ScriptAddress(uint32_t scriptNum_, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData, const DataAccess &access) : Script(scriptNum_, addressType, *std::get<0>(rawData), access), outputScript(std::get<0>(rawData)->getScript()) {
        auto inputPointer = std::get<1>(rawData);
        if (inputPointer != nullptr) {
            inputScript = inputPointer->getScript();
        } else {
            inputScript = ranges::nullopt;
        }
    }
    
    Nonstandard::ScriptAddress(uint32_t addressNum, const DataAccess &access) : Nonstandard(addressNum, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string Nonstandard::inputString() const {
        if (inputScript) {
            return ScriptToAsmStr(*inputScript);
        } else {
            return "No input script";
        }
    }
    
    std::string Nonstandard::outputString() const {
        return ScriptToAsmStr(outputScript);
    }
    
    std::string Nonstandard::toString() const {
        std::stringstream ss;
        ss << "NonStandardScript()";
        return ss.str();
    }
    
    std::string Nonstandard::toPrettyString() const {
        return toString();
    }
}
