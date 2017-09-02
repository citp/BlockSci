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

#include <unordered_map>

#include <sqlite3.h>

struct ParserConfiguration;

class AddressDB : public AddressTraverser {
    sqlite3 *db;
    bool firstRun;
    std::unordered_map<int, sqlite3_stmt *> insertStatements;
    
    AddressDB(std::pair<sqlite3 *, bool> init);
    
    void sawAddress(const blocksci::AddressPointer &pointer, uint32_t txNum) override;
    
    void linkP2SHAddress(const blocksci::AddressPointer &pointer, uint32_t txNum, uint32_t p2shNum) override;
    
public:
    AddressDB(const ParserConfiguration &config);
    ~AddressDB();
    
    void rollback(uint32_t maxTxIndex);
};

#endif /* address_db_h */
