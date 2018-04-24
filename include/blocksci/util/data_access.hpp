//
//  data_access.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_access_hpp
#define data_access_hpp

#include "data_configuration.hpp"

#include <blocksci/blocksci_export.h>

#include <memory>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    class AddressIndex;
    class HashIndex;
    class MempoolIndex;

    class BLOCKSCI_EXPORT DataAccess {
    public:
        DataConfiguration config;

        std::unique_ptr<ChainAccess> chain;
        std::unique_ptr<ScriptAccess> scripts;
        std::unique_ptr<AddressIndex> addressIndex;
        std::unique_ptr<HashIndex> hashIndex;
        std::unique_ptr<MempoolIndex> mempoolIndex;
        
        DataAccess();
        explicit DataAccess(DataConfiguration config_);
        DataAccess(DataAccess &&);
        DataAccess &operator=(DataAccess &&);
        ~DataAccess();

        const ChainAccess &getChain() const {
            return *chain;
        }

        const ScriptAccess &getScripts() const {
            return *scripts;
        }

        const MempoolIndex &getMempoolIndex() const {
            return *mempoolIndex;
        }

        AddressIndex &getAddressIndex() {
            return *addressIndex;
        }

        HashIndex &getHashIndex() {
            return *hashIndex;
        }
        
        operator DataConfiguration() const { return config; }
        
        void reload();
    };
}

#endif /* data_access_hpp */
