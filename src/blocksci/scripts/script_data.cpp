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
#include "bitcoin_script.hpp"

using namespace blocksci;

NonstandardScriptData::NonstandardScriptData(const CScriptView &scriptView) : scriptData(scriptView.size()) {}
NonstandardScriptData::NonstandardScriptData(const CScript &scriptView) : scriptData(scriptView.size()) {}
