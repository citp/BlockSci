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

AddressDB::AddressDB(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "addressDB") {
    rocksdb::Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    
    
    std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
    columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
    for (auto script : ScriptType::all) {
        columnDescriptors.push_back(rocksdb::ColumnFamilyDescriptor{scriptName(script), rocksdb::ColumnFamilyOptions{}});
    }
    rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandles, &db);
    assert(s.ok());
}

void AddressDB::tearDown() {
    for (auto handle : columnHandles) {
        delete handle;
    }
    delete db;
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
    auto column = columnHandles[static_cast<size_t>(ScriptType::SCRIPTHASH) + 1];
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
    rocksdb::Slice key(reinterpret_cast<const char *>(&scriptNum), sizeof(scriptNum));
    for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
        auto key = it->key();
        key.remove_prefix(sizeof(Address));
        OutputPointer outPoint;
        memcpy(&outPoint, key.data(), sizeof(outPoint));
        for (auto &a : addresses) {
            addAddress(a, outPoint);
        }
    }
    assert(it->status().ok());
    delete it;
}

void AddressDB::addAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    auto script = scriptType(address.type);
    std::array<rocksdb::Slice, 2> keyParts = {{
        rocksdb::Slice(reinterpret_cast<const char *>(&address), sizeof(address)),
        rocksdb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer))
    }};
    std::string sliceStr;
    rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
    db->Put(rocksdb::WriteOptions{}, columnHandles[static_cast<size_t>(script) + 1], key, rocksdb::Slice{});
}

void AddressDB::rollback(const blocksci::State &state) {
    for (auto script : ScriptType::all) {
        auto column = columnHandles[static_cast<size_t>(script) + 1];
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            key.remove_prefix(sizeof(Address));
            OutputPointer outPoint;
            memcpy(&outPoint, key.data(), sizeof(outPoint));
            if (outPoint.txNum >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::SCRIPTHASH)]) {
                db->Delete(rocksdb::WriteOptions(), column, it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
}
