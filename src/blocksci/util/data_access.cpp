//
//  data_access.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "util/data_access.hpp"
#include "util/data_configuration.hpp"

#include <blocksci/scripts/script_access.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

namespace blocksci {
    
    DataAccess::DataAccess(const DataConfiguration &config_, bool errorOnReorg_, BlockHeight blocksIgnored_) : config(config_), chain{std::make_unique<ChainAccess>(config, errorOnReorg_, blocksIgnored_)}, scripts{std::make_unique<ScriptAccess>(config)}, addressIndex{std::make_unique<AddressIndex>(config.addressDBFilePath().native())}, hashIndex{std::make_unique<HashIndex>(config.hashIndexFilePath().native(), true)} {}
    
    DataAccess &DataAccess::Instance(const DataConfiguration &config_, bool errorOnReorg, BlockHeight blocksIgnored) {
        // Since it's a static variable, if the class has already been created,
        // It won't be created again.
        // And it **is** thread-safe in C++11.
        
        static DataAccess myInstance(config_, errorOnReorg, blocksIgnored);
        static DataConfiguration config = config_;
        
        if (!config_.isNull()) {
            config = config_;
            myInstance.chain->reload();
            myInstance.scripts->reload();
        }
        
        // Return a reference to our instance.
        return myInstance;
    }
    
    DataAccess &DataAccess::Instance() {
        DataConfiguration config {};
        return Instance(config, true, BlockHeight{0});
    }
}



