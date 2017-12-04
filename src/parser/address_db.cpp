//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_access.hpp>

#include <unordered_set>
#include <string>
#include <iostream>

using namespace blocksci;

std::unordered_map<ScriptType::Enum,  lmdb::dbi> setupInsertStatements(lmdb::txn &wtxn) {
    std::unordered_map<ScriptType::Enum,  lmdb::dbi> scriptDbs;
    
    for (auto script : ScriptType::all) {
        scriptDbs.emplace(std::piecewise_construct, std::forward_as_tuple(script), std::forward_as_tuple(lmdb::dbi::open(wtxn, std::string(scriptName(script)).c_str(), MDB_CREATE | MDB_DUPSORT| MDB_DUPFIXED)));
    }
    return scriptDbs;
}

AddressDB::AddressDB(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "addressDB"), env(createAddressIndexEnviroment(path)), wtxn(lmdb::txn::begin(env)), scriptDbs(setupInsertStatements(wtxn)) {
}

//namespace {
//    void printStats(const MDB_stat &stat) {
//        std::cout << "ms_psize: " << stat.ms_psize << "\n";
//        std::cout << "ms_depth: " << stat.ms_depth << "\n";
//        std::cout << "ms_branch_pages: " << stat.ms_branch_pages << "\n";
//        std::cout << "ms_leaf_pages: " << stat.ms_leaf_pages << "\n";
//        std::cout << "ms_overflow_pages: " << stat.ms_overflow_pages << "\n";
//        std::cout << "ms_entries: " << stat.ms_entries << "\n";
//    }
//}



void AddressDB::tearDown() {
    
//    MDB_stat stats;
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::PUBKEY), &stats);
//    std::cout << "AddressDB\n";
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::PUBKEY), &stats);
//    std::cout << "PUBKEY:\n";
//    printStats(stats);
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::SCRIPTHASH), &stats);
//    std::cout << "SCRIPTHASH:\n";
//    printStats(stats);
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::MULTISIG), &stats);
//    std::cout << "MULTISIG:\n";
//    printStats(stats);
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::NONSTANDARD), &stats);
//    std::cout << "NONSTANDARD:\n";
//    printStats(stats);
//    dbi_stat(wtxn, scriptDbs.at(ScriptType::Enum::NULL_DATA), &stats);
//    std::cout << "NULL_DATA:\n";
//    printStats(stats);
//    std::cout << "\n";
    wtxn.commit();
}

void AddressDB::processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts) {
    uint16_t outputNum = 0;
    std::unordered_set<uint32_t> revealedScripts;
    
    for (auto input : tx.inputs()) {
        auto address = input.getAddress();
        
        std::vector<Address> addressesToRecurse;
        addressesToRecurse.push_back(address);
        while (addressesToRecurse.size() > 0) {
            std::vector<Address> addressesToAdd;
            bool insideP2SH = false;
            std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
                if (scriptType(a.type) == blocksci::ScriptType::Enum::SCRIPTHASH) {
                    auto p2sh = blocksci::script::ScriptHash{scripts, a.scriptNum};
                    if (!insideP2SH) {
                        if (p2sh.txRevealed < tx.txNum) {
                            return false;
                        } else if (revealedScripts.find(a.scriptNum) == revealedScripts.end()) {
                            assert(p2sh.txRevealed == tx.txNum);
                            revealedScripts.insert(a.scriptNum);
                            insideP2SH = true;
                            return true;
                        } else {
                            return false;
                        }
                    } else {
                        if (p2sh.txRevealed < tx.txNum) {
                            addressesToAdd.push_back(a);
                            return true;
                        } else if (revealedScripts.find(a.scriptNum) == revealedScripts.end()) {
                            assert(p2sh.txRevealed == tx.txNum);
                            revealedScripts.insert(a.scriptNum);
                            addressesToAdd.push_back(a);
                            addressesToRecurse.push_back(a);
                            return false;
                        } else {
                            return false;
                        }
                    }
                } else {
                    if (insideP2SH) {
                        addressesToAdd.push_back(a);
                    }
                    return insideP2SH;
                }
            };
            auto a = addressesToRecurse.back();
            addressesToRecurse.pop_back();
            visit(a, visitFunc, scripts);
            if (addressesToAdd.size() > 0) {
                revealedP2SH(a.scriptNum, addressesToAdd);
            }
        }
    }
    
    for (auto output : tx.outputs()) {
        auto address = output.getAddress();
        blocksci::OutputPointer pointer{tx.txNum, outputNum};
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            addAddress(a, pointer);
            // If address is p2sh then ignore the wrapped address if it was revealed after this transaction
            if (scriptType(a.type) == blocksci::ScriptType::Enum::SCRIPTHASH) {
                auto p2sh = blocksci::script::ScriptHash{scripts, a.scriptNum};
                if (!p2sh.hasBeenSpent() || tx.txNum < p2sh.txRevealed) {
                    return false;
                }
            }
            return true;
        };
        visit(address, visitFunc, scripts);
        outputNum++;
    }
}

void AddressDB::revealedP2SH(uint32_t scriptNum, const std::vector<Address> &addresses) {
    auto &dbi = scriptDbs.at(blocksci::ScriptType::SCRIPTHASH);
    auto cursor = lmdb::cursor::open(wtxn, dbi);
    lmdb::val key{&scriptNum, sizeof(scriptNum)};
    lmdb::val value;
    for (bool hasNext = cursor.get(key, value, MDB_SET_RANGE); hasNext; hasNext = cursor.get(key, value, MDB_NEXT)) {
        Address *address = key.data<Address>();
        if (address->scriptNum != scriptNum) {
            break;
        }
        OutputPointer *pointer = value.data<OutputPointer>();
        for (auto &a : addresses) {
            addAddress(a, *pointer);
        }
    }
}

void AddressDB::addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    auto script = scriptType(address.type);
    auto &dbi = scriptDbs.at(script);
    dbi.put(wtxn, address, pointer);
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
