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
#include "scripts/bitcoin_base58.hpp"
#include "address/address.hpp"

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
    }
    
    HashIndex::~HashIndex() {
        std::array<sqlite3_stmt *, 3> queryStatements{{txQuery, p2shQuery, pubkeyQuery}};
        for (auto &query : queryStatements) {
            sqlite3_finalize(query);
        }
        
        sqlite3_close(db);
    }
    
    std::vector<uint32_t> HashIndex::getMatches(sqlite3_stmt *stmt) const {
        std::vector<uint32_t> indexNums;
        int rc = 0;
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
            indexNums.push_back(txNum);
        }
        sqlite3_reset(stmt);
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
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) const {
        sqlite3_bind_blob(pubkeyQuery, 1, &pubkeyhash, sizeof(pubkeyhash), SQLITE_TRANSIENT);
        return getMatch(pubkeyQuery);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) const {
        sqlite3_bind_blob(p2shQuery, 1, &scripthash, sizeof(scripthash), SQLITE_TRANSIENT);
        return getMatch(p2shQuery);
    }
    
    // DecodeBase58
}
