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

#include <array>

namespace blocksci {
    HashIndex::HashIndex(const DataConfiguration &config) {
        auto rc = sqlite3_open(config.hashIndexFilePath().c_str(), &db);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        }
        
        std::array<sqlite3_stmt **, 3> queryStatements{{&txQuery, &p2shQuery, &pubkeyQuery}};
        
        size_t i = 0;
        for (auto &tableName : tableNames) {
            std::stringstream ss;
            ss << "SELECT BLOCKSCI_INDEX FROM " << tableName << " WHERE HASH_INDEX = ?" ;
            auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, queryStatements[i], nullptr);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            }
            i++;
        }
        
        std::array<sqlite3_stmt **, 3> prefixQueryStatements{{&txPrefixQuery, &p2shPrefixQuery, &pubkeyPrefixQuery}};
        
        i = 0;
        for (auto &tableName : tableNames) {
            std::stringstream ss;
            ss << "SELECT BLOCKSCI_INDEX FROM " << tableName << " WHERE HASH_INDEX >= ? AND HASH_INDEX < ?" ;
            auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, prefixQueryStatements[i], nullptr);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            }
            i++;
        }
    }
    
    HashIndex::~HashIndex() {
        std::array<sqlite3_stmt *, 3> queryStatements{{txQuery, p2shQuery, pubkeyQuery}};
        for (auto &query : queryStatements) {
            sqlite3_finalize(query);
        }
        
        std::array<sqlite3_stmt *, 3> prefixQueryStatements{{txPrefixQuery, p2shPrefixQuery, pubkeyPrefixQuery}};
        for (auto &query : prefixQueryStatements) {
            sqlite3_finalize(query);
        }
        
        sqlite3_close(db);
    }
    
    std::vector<uint32_t> HashIndex::getMatches(sqlite3_stmt *stmt) const {
        std::vector<uint32_t> indexNums;
        int rc = 0;
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = static_cast<uint32_t>(sqlite3_column_int(txQuery, 0));
            indexNums.push_back(txNum);
        }
        sqlite3_reset(txQuery);
        return indexNums;
    }
    
    uint32_t HashIndex::getMatch(sqlite3_stmt *stmt) const {
        auto matches = getMatches(stmt);
        if (matches.size() == 0) {
            return 0;
        } else {
            return matches[0];
        }
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) const {
        sqlite3_bind_blob(txQuery, 1, &txHash, sizeof(txHash), SQLITE_TRANSIENT);
        return getMatch(txQuery);
    }
    
    std::vector<uint32_t> HashIndex::getTxPrefixIndex(const uint256 &txHash) const {
        sqlite3_bind_blob(txQuery, 1, &txHash, sizeof(txHash), SQLITE_TRANSIENT);
        return getMatches(txQuery);
    }
    
    // DecodeBase58
}
