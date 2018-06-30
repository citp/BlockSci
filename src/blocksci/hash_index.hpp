//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <sqlite3.h>
#include <cstdint>
#include <stdio.h>

namespace blocksci {
    struct DataConfiguration;
    class uint256;
    
    class HashIndex {
        sqlite3 *db;
    public:
        HashIndex(const DataConfiguration &config);
        ~HashIndex();
        
        uint32_t getTxIndex(const uint256 &txHash) const;
    };
}

#endif /* hash_index_hpp */
