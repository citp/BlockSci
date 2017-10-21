//
//  address_db.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/27/17.
//
//

#ifndef address_db_h
#define address_db_h

#include "address_traverser.hpp"
#include "parser_fwd.hpp"

#include <blocksci/scripts/script_type.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>

#include <unordered_map>

class AddressDB : public AddressTraverser {
    SQLite::Database db;
    std::unordered_map<blocksci::ScriptType::Enum,  SQLite::Statement> insertStatements;
    SQLite::Transaction transaction;
    std::vector<blocksci::script::ScriptHash> p2shesToAdd;
    
    AddressDB(const ParserConfigurationBase &config, std::pair<sqlite3 *, bool> init);
    
    void addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
    
    void sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) override;
    void revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) override;
    
public:
    AddressDB(const ParserConfigurationBase &config);
    
    void rollback(const blocksci::State &state) override;
    void tearDown() override;
};

#endif /* address_db_h */
