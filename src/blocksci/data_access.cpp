//
//  data_access.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "data_access.hpp"
#include "data_configuration.hpp"

namespace blocksci {
    
    DataAccess::DataAccess(const DataConfiguration &config_, bool errorOnReorg_, uint32_t blocksIgnored_) : config(config_), chain(config, errorOnReorg_, blocksIgnored_), scripts(config), addressFirstSeen(config), addressIndex(config) {}
    
    DataAccess &DataAccess::Instance(const DataConfiguration &config_, bool errorOnReorg, uint32_t blocksIgnored) {
        // Since it's a static variable, if the class has already been created,
        // It won't be created again.
        // And it **is** thread-safe in C++11.
        
        static DataAccess myInstance(config_, errorOnReorg, blocksIgnored);
        static DataConfiguration config = config_;
        
        if (!config_.isNull() && config_ != config) {
            config = config_;
            myInstance.chain = ChainAccess(config, errorOnReorg, blocksIgnored);
            myInstance.scripts = ScriptAccess(config);
            myInstance.addressFirstSeen = AddressFirstSeenAccess(config);
            myInstance.addressIndex = AddressIndex(config);
        }
        
        // Return a reference to our instance.
        return myInstance;
    }
    
    DataAccess &DataAccess::Instance() {
        DataConfiguration config {};
        return Instance(config, true, 0);
    }
}



