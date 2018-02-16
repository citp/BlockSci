//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util/util.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <sstream>

constexpr int pubkeyHandleNum = 1;
constexpr int scriptHashHandleNum = 2;
constexpr int txHandleNum = 3;

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "hashIndex") {
    rocksdb::Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    
    
    std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
    columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
    columnDescriptors.emplace_back("P", rocksdb::ColumnFamilyOptions{});
    columnDescriptors.emplace_back("S", rocksdb::ColumnFamilyOptions{});
    columnDescriptors.emplace_back("T", rocksdb::ColumnFamilyOptions{});

    rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandles, &db);
    
    assert(s.ok());
    
}

void HashIndexCreator::tearDown() {
    for (auto handle : columnHandles) {
        delete handle;
    }
    delete db;
}

void HashIndexCreator::processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &) {
    auto hash = tx.getHash();
    rocksdb::Slice keySlice(reinterpret_cast<const char *>(&hash), sizeof(hash));
    rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&tx.txNum), sizeof(tx.txNum));
    db->Put(rocksdb::WriteOptions(), columnHandles[txHandleNum], keySlice, valueSlice);
}

void HashIndexCreator::processScript(const blocksci::Script &script, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    if (script.type == blocksci::ScriptType::Enum::PUBKEY) {
        blocksci::script::Pubkey pubkeyScript{scripts, script.scriptNum};
        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pubkeyScript.pubkeyhash), sizeof(pubkeyScript.pubkeyhash));
        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum));
        db->Put(rocksdb::WriteOptions(), columnHandles[pubkeyHandleNum], keySlice, valueSlice);
    } else if (script.type == blocksci::ScriptType::Enum::SCRIPTHASH) {
        blocksci::script::ScriptHash p2shScript{scripts, script.scriptNum};
        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&p2shScript.address), sizeof(p2shScript.address));
        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum));
        db->Put(rocksdb::WriteOptions(), columnHandles[scriptHashHandleNum], keySlice, valueSlice);
    }
}

void HashIndexCreator::rollback(const blocksci::State &state) {
    {
        auto column = columnHandles[pubkeyHandleNum];
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t value;
            memcpy(&value, it->value().data(), sizeof(value));
            if (value >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::PUBKEY)]) {
                db->Delete(rocksdb::WriteOptions(), column, it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
    
    {
        auto column = columnHandles[scriptHashHandleNum];
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t value;
            memcpy(&value, it->value().data(), sizeof(value));
            if (value >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::SCRIPTHASH)]) {
                db->Delete(rocksdb::WriteOptions(), column, it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
    
    {
        auto column = columnHandles[txHandleNum];
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t value;
            memcpy(&value, it->value().data(), sizeof(value));
            if (value >= state.txCount) {
                db->Delete(rocksdb::WriteOptions(), column, it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
}
