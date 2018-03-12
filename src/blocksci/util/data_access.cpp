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
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <unordered_set>

namespace blocksci {
    
    DataAccess::DataAccess(const DataConfiguration &config_) : config(config_), chain{std::make_unique<ChainAccess>(config)}, scripts{std::make_unique<ScriptAccess>(config)}, addressIndex{std::make_unique<AddressIndex>(config.addressDBFilePath().native(), true)}, hashIndex{std::make_unique<HashIndex>(config.hashIndexFilePath().native(), true)} {}
    
    DataAccess &DataAccess::Instance2(const DataConfiguration &config_) {
        // Since it's a static variable, if the class has already been created,
        // It won't be created again.
        // And it **is** thread-safe in C++11.
        
        static DataAccess myInstance(config_);
        static DataConfiguration config = config_;
        
        if (!config_.isNull()) {
            config = config_;
            myInstance.chain->reload();
            myInstance.scripts->reload();
        }
        
        // Return a reference to our instance.
        return myInstance;
    }
}



