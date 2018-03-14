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
    OpReturn::ScriptAddress(uint32_t scriptNum_, const DataAccess &access) : ScriptBase(scriptNum_, addressType, access), rawData(access.scripts->getScriptData<addressType>(scriptNum_)) {}
    
    std::string OpReturn::getData() const {
        return rawData->getData();
    }
    
    std::string OpReturn::toString() const {
        std::stringstream ss;
        ss << "NulldataAddressData()";
        return ss.str();
    }
    
    std::string OpReturn::toPrettyString() const {
        std::stringstream ss;
        ss << "NulldataAddressData(" << getData() << ")";
        return ss.str();
    }
}
