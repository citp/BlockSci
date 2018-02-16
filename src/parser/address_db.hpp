//
//  address_db.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/27/17.
//
//

#ifndef address_db_h
#define address_db_h

#include "parser_fwd.hpp"
#include "parser_index.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/scripts/script_type.hpp>

#include <rocksdb/db.h>

#include <unordered_map>

namespace blocksci {
    struct Address;
}

class AddressDB : public ParserIndex {
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts) override;
    void processScript(const blocksci::Script &, const blocksci::ChainAccess &, const blocksci::ScriptAccess &) override {}
    void addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
    void revealedP2SH(uint32_t scriptNum, const std::vector<blocksci::Address> &addresses);
    
public:
    AddressDB(const ParserConfigurationBase &config, const std::string &path);
    
    void rollback(const blocksci::State &state) override;
    void tearDown() override;
};

#endif /* address_db_h */
