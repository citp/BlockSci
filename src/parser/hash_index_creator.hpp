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
#include "parser_fwd.hpp"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include <tuple>

namespace blocksci {
    class ChainAccess;
    struct Transaction;
    class uint256;
}

class HashIndexCreator : public ParserIndex {
    SQLite::Database db;
    std::array<SQLite::Statement, 3> insertStatements;
    SQLite::Transaction transaction;
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    void processScript(const blocksci::ScriptPointer &pointer, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    void revealedP2SH(blocksci::script::ScriptHash &, const blocksci::ScriptAccess &) override {}
    
public:
    HashIndexCreator(const ParserConfigurationBase &config);
    void processTx(const blocksci::uint256 &hash, uint32_t index);
    void rollback(const blocksci::State &state) override;
    void tearDown() override;
};

#endif /* hash_index_creator_hpp */
