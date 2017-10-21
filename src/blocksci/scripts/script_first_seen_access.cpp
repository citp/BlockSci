//
//  script_first_seen_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "script_first_seen_access.hpp"
#include "data_configuration.hpp"

namespace blocksci {
    ScriptFirstSeenAccess::ScriptFirstSeenAccess(const DataConfiguration &config) :
    scriptFilesFirstSeen(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
        return config.firstSeenDirectory()/std::string{scriptName(tag)};
    })) {}
    
    void ScriptFirstSeenAccess::reload() {
        for_each(scriptFilesFirstSeen, [&](auto& file) -> decltype(auto) { file.reload(); });
    }
}
