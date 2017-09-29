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

#include <string_view>
#include <vector>
#include <cstdint>
#include <stdio.h>

namespace blocksci {
    using namespace std::string_view_literals;
    
    struct DataConfiguration;
    class uint256;
    class uint160;
    struct Address;
    
    class HashIndex {
        sqlite3 *db;
        sqlite3_stmt *pubkeyQuery;
        sqlite3_stmt *p2shQuery;
        sqlite3_stmt *txQuery;
        
        std::vector<uint32_t> getMatches(sqlite3_stmt * stmt) const;
        uint32_t getMatch(sqlite3_stmt * stmt) const;
        
    public:
        
        static constexpr auto txTableName = "TXHASH"sv;
        static constexpr auto p2shTableName = "P2SH_ADDRESS"sv;
        static constexpr auto pubkeyHashTableName = "PUBKEYHASH_ADDRESS"sv;
        
        static constexpr auto tableNames = {txTableName, p2shTableName, pubkeyHashTableName};
        
        HashIndex(const DataConfiguration &config);
        HashIndex(const HashIndex &index) = delete;
        HashIndex &operator=(const HashIndex &index) = delete;
        ~HashIndex();
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash) const;
        uint32_t getScriptHashIndex(const uint160 &scripthash) const;
        uint32_t getTxIndex(const uint256 &txHash) const;
    };
}

#endif /* hash_index_hpp */
