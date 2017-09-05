//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include "hash_index.hpp"
#include "data_configuration.hpp"
#include "bitcoin_uint256.hpp"

namespace blocksci {
    HashIndex::HashIndex(const DataConfiguration &config) {
        auto rc = sqlite3_open(config.hashIndexFilePath().c_str(), &db);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        }
    }
    
    HashIndex::~HashIndex() {
        sqlite3_close(db);
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) const {
        sqlite3_stmt *stmt;
        std::stringstream ss;
        ss << "SELECT BLOCKSCI_INDEX FROM " << "TXHASH" << " WHERE HASH_INDEX = ?" ;
        auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            return 0;
        }
        sqlite3_bind_blob(stmt, 1, &txHash, sizeof(txHash), SQLITE_TRANSIENT);
        
        std::vector<uint32_t> txNums;
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = sqlite3_column_int(stmt, 0);
            txNums.push_back(txNum);
        }
        sqlite3_finalize(stmt);
        if (txNums.size() == 0) {
            return 0;
        } else {
            return txNums[0];
        }
    }
}
