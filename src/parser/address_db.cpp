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
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
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
        for (auto script : ScriptType::all) {
            if (isDeduped(script)) {
                std::stringstream ss;
                ss << "CREATE TABLE ";
                ss << scriptName(script) << "(";
                ss << "ADDRESS_NUM     INT     NOT NULL,";
                ss << "ADDRESS_TYPE    TINYINT NOT NULL,";
                ss << "TX_INDEX        INT     NOT NULL,";
                ss << "OUTPUT_NUM      INT     NOT NULL);";
                
                char *zErrMsg = 0;
                auto rc = sqlite3_exec(addressDb, ss.str().c_str(), callback, 0, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    exit(1);
                }
            }
        }
    }
    return std::make_pair(addressDb, !dbAlreadyExists);
}

std::unordered_map<ScriptType::Enum,  sqlite3_stmt *> setupInsertStatements(sqlite3 *addressDb) {
    std::unordered_map<ScriptType::Enum,  sqlite3_stmt *> insertStatements;
    
    for (auto script : ScriptType::all) {
        if (isDeduped(script)) {
            sqlite3_stmt *stmt;
            std::stringstream ss;
            ss << "INSERT INTO " << scriptName(script) << " VALUES (?, ?, ?, ?)";
            auto rc = sqlite3_prepare_v2(addressDb,  ss.str().c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(addressDb));
            }
            insertStatements[script] = stmt;
        }
    }
    return insertStatements;
}

sqlite3_stmt *setupScriptHashQuery(sqlite3 *addressDb) {
    std::stringstream ss;
    ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(ScriptType::Enum::SCRIPTHASH) << " WHERE ADDRESS_NUM = ? AND TX_INDEX < ?" ;
    sqlite3_stmt *stmt;
    auto rc = sqlite3_prepare_v2(addressDb, ss.str().c_str(), -1, &stmt, 0);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(addressDb));
    }
    return stmt;
}

AddressDB::AddressDB(const ParserConfiguration &config) : AddressDB(config, openAddressDb(config.addressDBFilePath())) {}

AddressDB::AddressDB(const ParserConfiguration &config, std::pair<sqlite3 *, bool> init) : AddressTraverser(config, "addressDB"), db(init.first), firstRun(init.second), insertStatements(setupInsertStatements(db)), scriptHashQuery(setupScriptHashQuery(db)) {
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

AddressDB::~AddressDB() {
    preDestructor();
    for (auto &pair : insertStatements) {
        sqlite3_finalize(pair.second);
    }
    
    sqlite3_finalize(scriptHashQuery);
    
    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    
    if (firstRun) {
        for (auto script : ScriptType::all) {
            if (isDeduped(script)) {
                std::stringstream ss;
                ss << "CREATE INDEX ";
                ss << scriptName(script);
                ss << "_INDEX ON ";
                ss << scriptName(script);
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
    }
    
    sqlite3_close(db);
}

void AddressDB::revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) {
    sqlite3_bind_int(scriptHashQuery, 1, static_cast<int32_t>(scriptHash.scriptNum));
    sqlite3_bind_int64(scriptHashQuery, 2, static_cast<int64_t>(scriptHash.txRevealed));
    
    int rc = 0;
    while ( (rc = sqlite3_step(scriptHashQuery)) == SQLITE_ROW) {
        auto txNum = sqlite3_column_int64(scriptHashQuery, 0);
        auto outputNum = sqlite3_column_int(scriptHashQuery, 1);
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            blocksci::OutputPointer pointer{static_cast<uint32_t>(txNum), static_cast<uint16_t>(outputNum)};
            addAddress(a, pointer);
            return true;
        };
        visit(*scriptHash.getWrappedAddress(), visitFunc, scripts);
        
    }
    sqlite3_reset(scriptHashQuery);
}

void AddressDB::addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    auto script = scriptType(address.type);
    if (isDeduped(script)) {
        auto stmt = insertStatements[script];
        sqlite3_bind_int(stmt, 1, address.addressNum);
        sqlite3_bind_int(stmt, 2, static_cast<uint8_t>(address.type));
        sqlite3_bind_int(stmt, 3, pointer.txNum);
        sqlite3_bind_int(stmt, 4, pointer.inoutNum);
        
        auto rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
        }
        
        sqlite3_reset(stmt);
    }
}

void AddressDB::sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    addAddress(address, pointer);
}

void AddressDB::rollback(uint32_t maxTxIndex) {
    for (auto script : ScriptType::all) {
        if (isDeduped(script)) {
            std::stringstream ss;
            ss << "DELETE FROM " << scriptName(script) << " WHERE TX_INDEX >= " << maxTxIndex;
            sqlite3_exec(db,  ss.str().c_str(), NULL, NULL, NULL);
            ss.clear();
        }
    }
}
