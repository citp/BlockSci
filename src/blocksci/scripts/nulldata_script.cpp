//
//  nulldata_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "nulldata_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"

namespace blocksci {
    using namespace script;
    OpReturn::ScriptAddress(uint32_t scriptNum_, const RawData *raw, const ScriptAccess &access) : BaseScript(scriptNum_, scriptType, *raw, access), data(raw->getData()) {}
    
    OpReturn::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : OpReturn(addressNum, access.getScriptData<scriptType>(addressNum), access) {}
    
    std::string OpReturn::toString() const {
        std::stringstream ss;
        ss << "NulldataAddressData()";
        return ss.str();
    }
    
    std::string OpReturn::toPrettyString() const {
        return toString();
    }
}
