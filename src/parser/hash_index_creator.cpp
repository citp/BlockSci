//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"

#include <blocksci/hash_index.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_pointer.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <sstream>

std::pair<sqlite3 *, bool> openHashDb(boost::filesystem::path hashIndexFilePath);

static int callback(void *, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

std::pair<sqlite3 *, bool> openHashDb(boost::filesystem::path hashIndexFilePath) {
    
    sqlite3 *db;
    
    bool dbAlreadyExists = boost::filesystem::exists(hashIndexFilePath);
    
    /* Open database */
    int  rc = sqlite3_open_v2(hashIndexFilePath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    
    if (!dbAlreadyExists) {
        /* Create SQL statement */
        
        for (auto &table : blocksci::HashIndex::tableNames) {
            std::stringstream ss;
            ss << "CREATE TABLE ";
            ss << table << "(";
            ss <<
            "HASH_INDEX BLOB," \
            "BLOCKSCI_INDEX INT NOT NULL UNIQUE);";
            
            auto query = ss.str();
            rc = sqlite3_exec(db, query.c_str(), callback, 0, nullptr);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                exit(1);
            }
        }
    }
    return std::make_pair(db, !dbAlreadyExists);
}

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config): HashIndexCreator(config, openHashDb(config.hashIndexFilePath())) {
    
}

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config, std::pair<sqlite3 *, bool> init) : ParserIndex(config, "hashIndex"), db(init.first), firstRun(init.second) {
    
    std::array<sqlite3_stmt **, 3> insertStatements{{&txInsertStatement, &p2shInsertStatement, &pubkeyHashInsertStatement}};
    
    size_t i = 0;
    for (auto &tableName : blocksci::HashIndex::tableNames) {
        std::stringstream ss;
        ss << "INSERT INTO " << tableName << " VALUES (?, ?)";
        auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, insertStatements[i], nullptr);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        }
        i++;
    }
    
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}


void HashIndexCreator::tearDown() {
    if (firstRun) {
        for (auto &tableName : blocksci::HashIndex::tableNames) {
            std::stringstream ss;
            ss << "CREATE INDEX ";
            ss << tableName;
            ss << "_INDEX ON ";
            ss << tableName;
            ss << "(HASH_INDEX);";
            
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

HashIndexCreator::~HashIndexCreator() {
    sqlite3_finalize(txInsertStatement);
    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    sqlite3_close(db);
}

void HashIndexCreator::processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &) {
    auto hash = tx.getHash(chain);
    sqlite3_bind_blob(txInsertStatement, 1, &hash, sizeof(hash), SQLITE_TRANSIENT);
    sqlite3_bind_int(txInsertStatement, 2, static_cast<int>(tx.txNum));
    
    auto rc = sqlite3_step(txInsertStatement);
    if (rc != SQLITE_DONE) {
        printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
    }
    
    sqlite3_reset(txInsertStatement);
}

void HashIndexCreator::processScript(const blocksci::ScriptPointer &pointer, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    if (pointer.type == blocksci::ScriptType::Enum::PUBKEY) {
        blocksci::script::Pubkey pubkeyScript{scripts, pointer.scriptNum};
        sqlite3_bind_blob(pubkeyHashInsertStatement, 1, &pubkeyScript.pubkeyhash, sizeof(pubkeyScript.pubkeyhash), SQLITE_TRANSIENT);
        sqlite3_bind_int(pubkeyHashInsertStatement, 2, static_cast<int>(pointer.scriptNum));
        
        auto rc = sqlite3_step(pubkeyHashInsertStatement);
        if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
        }
        
        sqlite3_reset(pubkeyHashInsertStatement);
    } else if (pointer.type == blocksci::ScriptType::Enum::SCRIPTHASH) {
        blocksci::script::ScriptHash p2shScript{scripts, pointer.scriptNum};
        sqlite3_bind_blob(p2shInsertStatement, 1, &p2shScript.address, sizeof(p2shScript.address), SQLITE_TRANSIENT);
        sqlite3_bind_int(p2shInsertStatement, 2, static_cast<int>(pointer.scriptNum));
        
        auto rc = sqlite3_step(p2shInsertStatement);
        if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
        }
        
        sqlite3_reset(p2shInsertStatement);
    }
}
