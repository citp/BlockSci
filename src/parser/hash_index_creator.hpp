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

#include <blocksci/chain/chain_fwd.hpp>

#include <rocksdb/db.h>

#include <tuple>

namespace blocksci {
    class uint256;
}

class HashIndexCreator : public ParserIndex {
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts) override;
    void processScript(const blocksci::Script &script, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override {}

public:
    HashIndexCreator(const ParserConfigurationBase &config, const std::string &path);
    void processTx(const blocksci::uint256 &hash, uint32_t index);
    void rollback(const blocksci::State &state) override;
    void tearDown() override;
};

#endif /* hash_index_creator_hpp */
