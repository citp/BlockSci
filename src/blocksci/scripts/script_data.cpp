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

using namespace blocksci;

CScriptView NonstandardScriptData::getScript() const {
    return CScriptView(scriptData.begin(), scriptData.end());
}

CScriptView NonstandardSpendScriptData::getScript() const {
    return CScriptView(scriptData.begin(), scriptData.end());
}
