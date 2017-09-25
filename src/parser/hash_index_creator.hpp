//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#ifndef hash_index_creator_hpp
#define hash_index_creator_hpp

#include "parser_index.hpp"

#include <sqlite3.h>

#include <tuple>
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    struct Transaction;
    class uint256;
}

struct ParserConfiguration;

class HashIndex : public ParserIndex {
    sqlite3 *db;
    sqlite3_stmt *txInsertStatement;
    sqlite3_stmt *pubkeyHashInsertStatement;
    sqlite3_stmt *p2shInsertStatement;
    bool firstRun;
    
    HashIndex(const ParserConfiguration &config, std::pair<sqlite3 *, bool> init);
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    void processScript(const blocksci::ScriptPointer &pointer, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    void revealedP2SH(blocksci::script::ScriptHash &, const blocksci::ScriptAccess &) override {}
    void tearDown() override;
public:
    HashIndex(const ParserConfiguration &config);
    ~HashIndex();
    void processTx(const blocksci::uint256 &hash, uint32_t index);
};

#endif /* hash_index_creator_hpp */
