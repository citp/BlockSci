//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/address/address_index.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/scripts/scripts.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <string>

using namespace blocksci;

std::unordered_map<ScriptType::Enum,  lmdb::dbi> setupInsertStatements(lmdb::txn &wtxn) {
    std::unordered_map<ScriptType::Enum,  lmdb::dbi> scriptDbs;
    
    for (auto script : ScriptType::all) {
        scriptDbs.emplace(std::piecewise_construct, std::forward_as_tuple(script), std::forward_as_tuple(lmdb::dbi::open(wtxn, std::string(scriptName(script)).c_str(), MDB_CREATE | MDB_DUPSORT| MDB_DUPFIXED)));
    }
    return scriptDbs;
}

AddressDB::AddressDB(const std::string &path) : AddressTraverser(config, "addressDB"), env(createAddressIndexEnviroment(path)), wtxn(lmdb::txn::begin(env)), scriptDbs(setupInsertStatements(wtxn)) {
}

void AddressDB::tearDown() {    
    wtxn.commit();
}

void AddressDB::revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) {
    auto &dbi = scriptDbs.at(blocksci::ScriptType::SCRIPTHASH);
    auto cursor = lmdb::cursor::open(wtxn, dbi);
    lmdb::val key{&scriptHash.scriptNum, sizeof(scriptHash.scriptNum)};
    lmdb::val value;
    for (bool hasNext = cursor.get(key, value, MDB_SET_RANGE); hasNext; hasNext = cursor.get(key, value, MDB_NEXT)) {
        Address *address = key.data<Address>();
        if (address->scriptNum != scriptHash.scriptNum) {
            break;
        }
        OutputPointer *pointer = value.data<OutputPointer>();
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            addAddress(a, *pointer);
            return true;
        };
        address->getScript(scripts).visitPointers(visitFunc);
    }
}

void AddressDB::addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    auto script = scriptType(address.type);
    auto &dbi = scriptDbs.at(script);
    dbi.put(wtxn, address, pointer);
}

void AddressDB::sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    addAddress(address, pointer);
}

void AddressDB::rollback(const blocksci::State &state) {
    for (auto script : ScriptType::all) {
        auto &dbi = scriptDbs.at(script);
        auto cursor = lmdb::cursor::open(wtxn, dbi);
        lmdb::val key, value;
        while (cursor.get(key, value, MDB_NEXT)) {
            blocksci::OutputPointer *pointer = value.data<blocksci::OutputPointer>();
            if (pointer->txNum >= state.txCount) {
                lmdb::cursor_del(cursor.handle());
            }
        }
    }
}
