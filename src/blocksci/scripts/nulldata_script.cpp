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
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    using namespace script;
    OpReturn::ScriptAddress(uint32_t scriptNum_, const RawData *raw, const DataAccess &access) : Script(scriptNum_, addressType, *raw, access), data(raw->getData()) {}
    
    OpReturn::ScriptAddress(uint32_t addressNum, const DataAccess &access) : OpReturn(addressNum, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
    std::string OpReturn::toString() const {
        std::stringstream ss;
        ss << "NulldataAddressData()";
        return ss.str();
    }
    
    std::string OpReturn::toPrettyString() const {
        return toString();
    }
}
