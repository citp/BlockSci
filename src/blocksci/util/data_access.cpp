//
//  data_access.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "data_access.hpp"
#include "data_configuration.hpp"

#include <blocksci/scripts/script_access.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <unordered_set>

namespace blocksci {
    
    DataAccess::DataAccess(DataConfiguration config_) : config(std::move(config_)), chain{std::make_unique<ChainAccess>(config)}, scripts{std::make_unique<ScriptAccess>(config)}, addressIndex{std::make_unique<AddressIndex>(config.addressDBFilePath().native(), true)}, hashIndex{std::make_unique<HashIndex>(config.hashIndexFilePath().native(), true)} {}
}



