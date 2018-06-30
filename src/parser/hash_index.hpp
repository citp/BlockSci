//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <sqlite3.h>

#include <tuple>
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    struct Transaction;
}

struct ParserConfiguration;

class HashIndex {
    sqlite3 *db;
    sqlite3_stmt *txInsertStatement;
    sqlite3_stmt *pubkeyHashInsertStatement;
    sqlite3_stmt *p2shInsertStatement;
    bool firstRun;
    
    HashIndex(std::pair<sqlite3 *, bool> init);
    
public:
    HashIndex(const ParserConfiguration &config);
    ~HashIndex();
    void processTx(const blocksci::ChainAccess &chain, const blocksci::Transaction &tx);
};

#endif /* tx_hash_index_hpp */
