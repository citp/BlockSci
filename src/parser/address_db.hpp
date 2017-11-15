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

#include <lmdbxx/lmdb++.h>

#include <unordered_map>

class AddressDB : public AddressTraverser {
    lmdb::env env;
    lmdb::txn wtxn;
    
    std::unordered_map<blocksci::ScriptType::Enum,  lmdb::dbi> scriptDbs;
    
    void addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
    
    void sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) override;
    void revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) override;
    
public:
    AddressDB(const std::string &path);
    
    void rollback(const blocksci::State &state) override;
    void tearDown() override;
};

#endif /* address_db_h */
