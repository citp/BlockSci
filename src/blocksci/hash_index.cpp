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
#include "util.hpp"

#include <array>

namespace blocksci {
    
    template<HashIndex::IndexType::Enum type>
    struct QueryFunctor {
        static SQLite::Statement f(SQLite::Database &db) {
            std::stringstream ss;
            ss << "SELECT BLOCKSCI_INDEX FROM " << HashIndex::IndexType::tableNames[static_cast<size_t>(type)] << " WHERE HASH_INDEX = ?" ;
            return SQLite::Statement{db, ss.str()};
        }
    };
    
    HashIndex::HashIndex(const DataConfiguration &config) : db(config.hashIndexFilePath().native()), queries(make_static_table<IndexType, QueryFunctor>(db)) {}
    
    std::vector<uint32_t> getMatches(SQLite::Statement &query) {
        std::vector<uint32_t> indexNums;
        while (query.executeStep()) {
            auto txNum = static_cast<uint32_t>(query.getColumn(0).getInt());
            indexNums.push_back(txNum);
        }
        query.reset();
        return indexNums;
    }
    
    uint32_t getMatch(SQLite::Statement &query) {
        auto matches = getMatches(query);
        if (matches.size() == 0) {
            return 0;
        } else {
            return matches[0];
        }
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) const {
        auto &query = queries[static_cast<size_t>(IndexType::Tx)];
        query.bind(1, reinterpret_cast<const void *>(&txHash), sizeof(txHash));
        return getMatch(query);
    }
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) const {
        auto &query = queries[static_cast<size_t>(IndexType::PubkeyHash)];
        query.bind(1, reinterpret_cast<const void *>(&pubkeyhash), sizeof(pubkeyhash));
        return getMatch(query);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) const {
        auto &query = queries[static_cast<size_t>(IndexType::ScriptHash)];
        query.bind(1, reinterpret_cast<const void *>(&scripthash), sizeof(scripthash));
        return getMatch(query);
    }
}
