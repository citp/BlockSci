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

#include <memory>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    class AddressIndex;
    class HashIndex;
    class MempoolIndex;

    /** This class wraps and manages all data and index access classes
     *     - ChainAccess: Provides data access for blocks, transactions, inputs, and outputs
     *     - ScriptAccess: Provides data access for script data of all address types
     *     - AddressIndex: Provides data access to address indexes (RocksDB database)
     *     - HashIndex: Provides data access to hash indexes (RocksDB database)
     *     - MempoolIndex: Provides data access to the mempool index (when a transaction has been first seen)
     *
     *     - DataConfiguration: Loads and holds blockchain configuration files, needed to load blockchains
     */
    class DataAccess {
    public:
        /** Loads and holds blockchain configuration files, needed to load blockchains */
        DataConfiguration config;

        /** Provides data access for blocks, transactions, inputs, and outputs
         *
         * The files here represent the core data about blocks and transactions.
         * Data is stored in a hybrid column and row based structure.
         * The core tx_data.dat and tx_index.dat are the core data files containing most information that is
         * used about transactions, inputs, and outputs. The other files in this folder are column oriented storing
         * less frequently accessed values.
         *
         * Directory: chain/
         */
        std::unique_ptr<ChainAccess> chain;

        /** Provides access to script data of all address types
         *
         * BlockSci supports the parsing of all standard Bitcoin address types in order to extract relevant data.
         * Each address type has its own file(s) storing this data.
         *
         * Directory: scripts/
         */
        std::unique_ptr<ScriptAccess> scripts;

        /** Provides access to address indexes (RocksDB database)
         *
         * This RocksDB database stores information about which outputs a given address
         * is used in as well as information about how different addresses relate to each other.
         *
         * Directory: addressesDb/
         */
        std::unique_ptr<AddressIndex> addressIndex;

        /** Provides access to hash indexes (RocksDB database)
         *
         * This RocksDB database is a lookup table from tx hash and address hash to internal BlockSci index for those objects.
         *
         * Directory: hashIndex/
         */
        std::unique_ptr<HashIndex> hashIndex;

        /** Provides access to the mempool index, which stores the timestamp of when a transaction has been
         * first seen. Only relevant when BlockSci's mempool_recorder is enabled (= running).
         *
         * Directory: mempool/
         */
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
