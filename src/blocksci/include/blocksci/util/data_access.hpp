//
//  data_access.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_access_hpp
#define data_access_hpp

#include <blocksci/blocksci_export.h>
#include "data_configuration.hpp"

#include <blocksci/core/chain_access.hpp>
#include <blocksci/core/script_access.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/index/mempool_index.hpp>

namespace blocksci {
    class BLOCKSCI_EXPORT DataAccess {
    public:
        DataConfiguration config;
        ChainAccess chain;
        ScriptAccess scripts;
        AddressIndex addressIndex;
        HashIndex hashIndex;
        MempoolIndex mempoolIndex;
        
        DataAccess() = default;
        explicit DataAccess(DataConfiguration config_) : config(std::move(config_)), chain{config}, scripts{config}, addressIndex{config.addressDBFilePath().native(), true}, hashIndex{config.hashIndexFilePath().native(), true}, mempoolIndex{config} {}
        
        operator DataConfiguration() const { return config; }
        
        void reload() {
            chain.reload();
            scripts.reload();
            mempoolIndex.reload();
        }
    };
}

#endif /* data_access_hpp */
