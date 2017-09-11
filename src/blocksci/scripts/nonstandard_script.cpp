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

namespace blocksci {
    using namespace script;
    Nonstandard::ScriptAddress(std::tuple<const NonstandardScriptData *, const NonstandardScriptData *> &&rawData) : outputScript(std::get<0>(rawData)->getScript()) {
        auto inputPointer = std::get<1>(rawData);
        if (inputPointer != nullptr) {
            inputScript = inputPointer->getScript();
        } else {
            inputScript = boost::none;
        }
    }
    
    Nonstandard::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Nonstandard(access.getScriptData<ScriptType::Enum::NONSTANDARD>(addressNum)) {}
    
    bool Nonstandard::operator==(const Script &other) {
        auto otherA = dynamic_cast<const Nonstandard *>(&other);
        return otherA && otherA->inputString() == inputString() && otherA->outputString() == outputString();
    }
    
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
    
    std::string Nonstandard::toString(const DataConfiguration &) const {
        std::stringstream ss;
        ss << "NonStandardScript()";
        return ss.str();
    }
    
    std::string Nonstandard::toPrettyString(const DataConfiguration &config, const ScriptAccess &) const {
        return toString(config);
    }
}
