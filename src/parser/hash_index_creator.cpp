//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util.hpp>
#include <blocksci/hash_index.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_pointer.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <sstream>

std::pair<sqlite3 *, bool> openHashDb(boost::filesystem::path hashIndexFilePath);

template<blocksci::HashIndex::IndexType::Enum type>
struct InsertFunctor {
    static SQLite::Statement f(SQLite::Database &db) {
        std::stringstream ss;
        ss << "INSERT INTO " << blocksci::HashIndex::IndexType::tableNames[static_cast<size_t>(type)] << " VALUES (?, ?)";
        return SQLite::Statement{db, ss.str()};
    }
};

SQLite::Database createDB(const ParserConfigurationBase &config) {
    SQLite::Database db{config.hashIndexFilePath().native(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE};
    for (auto &table : blocksci::HashIndex::IndexType::tableNames) {
        if (!db.tableExists(std::string(table))) {
            std::stringstream ss;
            ss << "CREATE TABLE ";
            ss << table << "(";
            ss <<
            "HASH_INDEX BLOB," \
            "BLOCKSCI_INDEX INT NOT NULL UNIQUE);";
            
            db.exec(ss.str());
        }
    }
    return {config.hashIndexFilePath().native(), SQLite::OPEN_READWRITE};
}

std::array<SQLite::Statement, blocksci::HashIndex::IndexType::size> setupHashInsertStatements(SQLite::Database &db) {
    db.exec("PRAGMA synchronous = OFF");
    return blocksci::make_static_table<blocksci::HashIndex::IndexType, InsertFunctor>(db);
}

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config) : ParserIndex(config, "hashIndex"), db(createDB(config)), insertStatements(setupHashInsertStatements(db)), transaction(db) {
    
    
}

void HashIndexCreator::tearDown() {
    for (auto &tableName : blocksci::HashIndex::IndexType::tableNames) {
        std::stringstream ss;
        ss << "CREATE INDEX IF NOT EXISTS ";
        ss << tableName;
        ss << "_INDEX ON ";
        ss << tableName;
        ss << "(HASH_INDEX);";
        db.exec(ss.str());
    }
    transaction.commit();
}

void HashIndexCreator::processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &) {
    auto hash = tx.getHash(chain);
    auto &query = insertStatements[static_cast<size_t>(blocksci::HashIndex::IndexType::Tx)];
    query.bind(1, reinterpret_cast<const void *>(&hash), sizeof(hash));
    query.bind(2, tx.txNum);
    query.executeStep();
    query.reset();
}

void HashIndexCreator::processScript(const blocksci::ScriptPointer &pointer, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    if (pointer.type == blocksci::ScriptType::Enum::PUBKEY) {
        blocksci::script::Pubkey pubkeyScript{scripts, pointer.scriptNum};
        auto &query = insertStatements[static_cast<size_t>(blocksci::HashIndex::IndexType::PubkeyHash)];
        query.bind(1, reinterpret_cast<const void *>(&pubkeyScript.pubkeyhash), sizeof(pubkeyScript.pubkeyhash));
        query.bind(2, pointer.scriptNum);
        query.executeStep();
        query.reset();
    } else if (pointer.type == blocksci::ScriptType::Enum::SCRIPTHASH) {
        blocksci::script::ScriptHash p2shScript{scripts, pointer.scriptNum};
        auto &query = insertStatements[static_cast<size_t>(blocksci::HashIndex::IndexType::ScriptHash)];
        query.bind(1, reinterpret_cast<const void *>(&p2shScript.address), sizeof(p2shScript.address));
        query.bind(2, pointer.scriptNum);
        query.executeStep();
        query.reset();
    }
}

void HashIndexCreator::rollback(const blocksci::State &state) {
    std::stringstream ss;
    ss << "DELETE FROM " << blocksci::HashIndex::IndexType::tableNames[static_cast<size_t>(blocksci::HashIndex::IndexType::PubkeyHash)] << " WHERE BLOCKSCI_INDEX >= " << state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::PUBKEY)];
    db.exec(ss.str());
    ss.clear();
    
    ss << "DELETE FROM " << blocksci::HashIndex::IndexType::tableNames[static_cast<size_t>(blocksci::HashIndex::IndexType::ScriptHash)] << " WHERE BLOCKSCI_INDEX >= " << state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::SCRIPTHASH)];
    db.exec(ss.str());
    ss.clear();
    
    ss << "DELETE FROM " << blocksci::HashIndex::IndexType::tableNames[static_cast<size_t>(blocksci::HashIndex::IndexType::Tx)] << " WHERE BLOCKSCI_INDEX >= " << state.txCount;
    db.exec(ss.str());
    ss.clear();
}
