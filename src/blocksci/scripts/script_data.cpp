//
//  script_data.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_data.hpp"
#include "script_view.hpp"
#include <blocksci/util/hash.hpp>

namespace blocksci {
    uint160 ScriptHashData::getHash160() const {
        if (isSegwit) {
            return ripemd160(reinterpret_cast<const char *>(&hash256), sizeof(hash256));
        } else {
            return hash160;
        }
    }
    
    CScriptView NonstandardScriptData::getScript() const {
        return {scriptData.begin(), scriptData.end()};
    }
    
    CScriptView NonstandardSpendScriptData::getScript() const {
        return {scriptData.begin(), scriptData.end()};
    }

}
