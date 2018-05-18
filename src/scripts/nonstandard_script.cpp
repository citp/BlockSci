//
//  nonstandard_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/nonstandard_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>
#include <internal/script_view.hpp>

#include <sstream>

namespace blocksci {

    ScriptAddress<AddressType::NONSTANDARD>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    ranges::optional<CScriptView> ScriptAddress<AddressType::NONSTANDARD>::getInputScript() const {
        if (rawInputData != nullptr) {
            return CScriptView{rawInputData->scriptData.begin(), rawInputData->scriptData.end()};
        } else {
            return ranges::nullopt;
        }
    }
    
    CScriptView ScriptAddress<AddressType::NONSTANDARD>::getOutputScript() const {
        return {getData()->scriptData.begin(), getData()->scriptData.end()};
    }

    std::string ScriptAddress<AddressType::NONSTANDARD>::inputString() const {
        auto inputScript = getInputScript();
        if (inputScript) {
            return ScriptToAsmStr(*inputScript);
        } else {
            return "No input script";
        }
    }
    
    std::string ScriptAddress<AddressType::NONSTANDARD>::outputString() const {
        return ScriptToAsmStr(getOutputScript());
    }
    
    std::string ScriptAddress<AddressType::NONSTANDARD>::toString() const {
        std::stringstream ss;
        ss << "NonStandardScript()";
        return ss.str();
    }
    
    std::string ScriptAddress<AddressType::NONSTANDARD>::toPrettyString() const {
        std::stringstream ss;
        ss << "NonStandardScript(" << inputString() << ", " << outputString() << ")";
        return ss.str();
    }
} // namespace blocksci
