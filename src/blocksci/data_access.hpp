//
//  data_access.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_access_hpp
#define data_access_hpp

#include "scripts/script_access.hpp"
#include "scripts/script_first_seen_access.hpp"
#include "chain/chain_access.hpp"
#include "data_configuration.hpp"

#include <stdio.h>

namespace blocksci {

    class DataAccess {
    public:
        const DataConfiguration config;
        ChainAccess chain;
        ScriptAccess scripts;
        ScriptFirstSeenAccess scriptsFirstSeen;
        
        static DataAccess &Instance(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        static DataAccess &Instance();
        
        DataAccess(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        
        // delete copy and move constructors and assign operators
        DataAccess(DataAccess const&) = delete;             // Copy construct
        DataAccess(DataAccess&&) = delete;                  // Move construct
        DataAccess& operator=(DataAccess const&) = delete;  // Copy assign
        DataAccess& operator=(DataAccess &&) = delete;      // Move assign
        
    };
}

#endif /* data_access_hpp */
