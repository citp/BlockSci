//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"

#include "parser_configuration.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/address/address_index.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/filesystem.hpp>

#include <string>

using namespace blocksci;

static int callback(void *, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

std::pair<sqlite3 *, bool> openAddressDb(boost::filesystem::path addressesDBFilePath) {
    
    sqlite3 *addressDb;
    
    bool dbAlreadyExists = boost::filesystem::exists(addressesDBFilePath);
    
    /* Open database */
    int  rc = sqlite3_open_v2(addressesDBFilePath.c_str(), &addressDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        exit(1);
    }
    
    if (!dbAlreadyExists) {
        /* Create SQL statement */
        
        for (auto &tableName : AddressIndex::addrTables) {
            std::stringstream ss;
            ss << "CREATE TABLE ";
            ss << tableName << "(";
            ss <<
            "ADDRESS_NUM     INT     NOT NULL," \
            "TX_INDEX        INT     NOT NULL);";
            
            char *zErrMsg = 0;
            auto rc = sqlite3_exec(addressDb, ss.str().c_str(), callback, 0, &zErrMsg);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                exit(1);
            }
        }
    }
    return std::make_pair(addressDb, !dbAlreadyExists);
}

std::unordered_map<int, sqlite3_stmt *> setupInsertStatements(sqlite3 *addressDb) {
    std::unordered_map<int, sqlite3_stmt *> insertStatements;
    
    std::unordered_map<int, std::string> tableNames;
    tableNames[0] = "SINGLE_ADDRESSES";
    tableNames[1] = "MULTISIG_ADDRESSES";
    tableNames[2] = "P2SH_ADDRESSES";
    
    for (auto &pair : tableNames) {
        sqlite3_stmt *stmt;
        auto rc = sqlite3_prepare_v2(addressDb,  ("INSERT INTO " + pair.second + " VALUES (?, ?)").c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(addressDb));
        }
        insertStatements[pair.first] = stmt;
    }
    return insertStatements;
}

AddressDB::AddressDB(const ParserConfiguration &config) : AddressDB(openAddressDb(config.addressDBFilePath())) {}

AddressDB::AddressDB(std::pair<sqlite3 *, bool> init) : db(init.first), firstRun(init.second), insertStatements(setupInsertStatements(db)) {
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

AddressDB::~AddressDB() {
    for (auto &pair : insertStatements) {
        sqlite3_finalize(pair.second);
    }
    
    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    
    if (firstRun) {
        
        for (auto &tableName : AddressIndex::addrTables) {
            std::stringstream ss;
            ss << "CREATE INDEX ";
            ss << tableName;
            ss << "_INDEX ON ";
            ss << tableName;
            ss << "(ADDRESS_NUM);";
            
            char *zErrMsg = 0;
            auto rc = sqlite3_exec(db, ss.str().c_str(), callback, 0, &zErrMsg);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                exit(1);
            }
        }
        
    }
    
    sqlite3_close(db);
}

void AddressDB::sawAddress(const blocksci::Address &pointer, uint32_t txNum) {
    auto type = pointer.getDBType();

    if (type >= 0 && type < 3) {
        auto stmt = insertStatements[type];
        sqlite3_bind_int(stmt, 1, pointer.addressNum);
        sqlite3_bind_int(stmt, 2, txNum);
        
        auto rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
        }
        
        sqlite3_reset(stmt);
    }
}

void AddressDB::linkP2SHAddress(const blocksci::Address &, uint32_t, uint32_t) {
    
}

void AddressDB::rollback(uint32_t maxTxIndex) {
    std::stringstream ss;
    ss << "DELETE FROM SINGLE_ADDRESSES WHERE TX_INDEX >= " << maxTxIndex;
    sqlite3_exec(db,  ss.str().c_str(), NULL, NULL, NULL);
    ss.clear();
    ss << "DELETE FROM MULTISIG_ADDRESSES WHERE TX_INDEX >= " << maxTxIndex;
    sqlite3_exec(db,  ss.str().c_str(), NULL, NULL, NULL);
    ss.clear();
    ss << "DELETE FROM P2SH_ADDRESSES WHERE TX_INDEX >= " << maxTxIndex;
    sqlite3_exec(db,  ss.str().c_str(), NULL, NULL, NULL);
    ss.clear();
}
