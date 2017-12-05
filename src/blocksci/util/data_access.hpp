//
//  data_access.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_access_hpp
#define data_access_hpp

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/util/data_configuration.hpp>

namespace blocksci {
    class AddressIndex;

    class DataAccess {
    public:
        const DataConfiguration config;
        std::unique_ptr<ChainAccess> chain;
        std::unique_ptr<ScriptAccess> scripts;
        std::unique_ptr<AddressIndex> addressIndex;
        std::unique_ptr<HashIndex> hashIndex;
        
        static DataAccess &Instance(const DataConfiguration &config, bool errorOnReorg, BlockHeight blocksIgnored);
        static DataAccess &Instance();
        
        DataAccess(const DataConfiguration &config, bool errorOnReorg, BlockHeight blocksIgnored);
        
        // delete copy and move constructors and assign operators
        DataAccess(DataAccess const&) = delete;             // Copy construct
        DataAccess(DataAccess&&) = delete;                  // Move construct
        DataAccess& operator=(DataAccess const&) = delete;  // Copy assign
        DataAccess& operator=(DataAccess &&) = delete;      // Move assign
        
    };
}

#endif /* data_access_hpp */
