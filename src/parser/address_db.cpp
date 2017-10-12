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
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/optional/optional.hpp>

#include <string>

using namespace blocksci;

SQLite::Database openAddressDb(boost::filesystem::path addressesDBFilePath) {
    SQLite::Database db{addressesDBFilePath.native(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE};
    
    for (auto script : ScriptType::all) {
        if (!db.tableExists(std::string(scriptName(script)))) {
            std::stringstream ss;
            ss << "CREATE TABLE ";
            ss << scriptName(script) << "(";
            ss << "ADDRESS_NUM     INT     NOT NULL,";
            ss << "ADDRESS_TYPE    TINYINT NOT NULL,";
            ss << "TX_INDEX        INT     NOT NULL,";
            ss << "OUTPUT_NUM      INT     NOT NULL);";
            db.exec(ss.str());
        }
    }
    return {addressesDBFilePath.native(), SQLite::OPEN_READWRITE};
}

std::unordered_map<ScriptType::Enum,  SQLite::Statement> setupInsertStatements(SQLite::Database &db) {
    std::unordered_map<ScriptType::Enum,  SQLite::Statement> insertStatements;
    
    for (auto script : ScriptType::all) {
        std::stringstream ss;
        ss << "INSERT INTO " << scriptName(script) << " VALUES (?, ?, ?, ?)";
        insertStatements.emplace(std::piecewise_construct, std::forward_as_tuple(script), std::forward_as_tuple(db, ss.str()));
    }
    db.exec("PRAGMA synchronous = OFF");
    return insertStatements;
}

AddressDB::AddressDB(const ParserConfigurationBase &config) : AddressTraverser(config, "addressDB"), db(openAddressDb(config.addressDBFilePath())), insertStatements(setupInsertStatements(db)), transaction(db) {
}

void AddressDB::tearDown() {
    for (auto script : ScriptType::all) {
        std::stringstream ss;
        ss << "CREATE INDEX IF NOT EXISTS ";
        ss << scriptName(script);
        ss << "_INDEX ON ";
        ss << scriptName(script);
        ss << "(ADDRESS_NUM);";
        
        db.exec(ss.str());
    }
    
    std::stringstream ss;
    ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(ScriptType::Enum::SCRIPTHASH) << " WHERE ADDRESS_NUM = ? AND TX_INDEX < ?" ;
    SQLite::Statement query{db, ss.str()};
    blocksci::ScriptAccess scripts{config};
    
    for (auto &scriptHash : p2shesToAdd) {
        query.bind(1, scriptHash.scriptNum);
        query.bind(2, scriptHash.txRevealed);
        while (query.executeStep()) {
            auto txNum = query.getColumn(0).getUInt();
            auto outputNum = static_cast<uint16_t>(query.getColumn(1).getUInt());
            blocksci::OutputPointer pointer{txNum, outputNum};
            std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
                addAddress(a, pointer);
                return true;
            };
        }
        query.reset();
    }
    
    transaction.commit();
}

void AddressDB::revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &) {
    p2shesToAdd.push_back(scriptHash);
}

void AddressDB::addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    auto script = scriptType(address.type);
    auto &insert = insertStatements.at(script);
    insert.bind(1, address.addressNum);
    insert.bind(2, static_cast<uint32_t>(address.type));
    insert.bind(3, pointer.txNum);
    insert.bind(4, static_cast<uint32_t>(pointer.inoutNum));
    insert.exec();
    insert.reset();
}

void AddressDB::sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    addAddress(address, pointer);
}

void AddressDB::rollback(const blocksci::State &state) {
    for (auto script : ScriptType::all) {
        std::stringstream ss;
        ss << "DELETE FROM " << scriptName(script) << " WHERE TX_INDEX >= " << state.txCount;
        db.exec(ss.str());
    }
}
