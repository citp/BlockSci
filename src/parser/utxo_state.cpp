//
//  utxo_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "utxo_state.hpp"
#include "parser_configuration.hpp"

#include <leveldb/write_batch.h>
#include <iostream>

const auto DeletedKeysMaxSize = 5'000'000;
const auto UTXOMapMaxSize = 10'000'000;

UTXOState::UTXOState(const ParserConfiguration &config_) : config(config_)  {
    leveldb::Options options;
    options.create_if_missing = true;
    options.max_open_files = 64;
    leveldb::DB::Open(options, config.utxoDBPath().c_str(), &levelDb);
    
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    RawOutputPointer deletedPointer = {nullHash, 0};
    utxoMap.set_deleted_key(deletedPointer);
    oldUTXOMap.set_deleted_key(deletedPointer);
}

UTXOState::~UTXOState() {
    clearDeletedKeys();
    clearUTXOCache();
    clearUTXOCache();
    
    if (utxoClearFuture.valid()) {
        utxoClearFuture.get();
    }
}


UTXO UTXOState::spendOutput(const RawOutputPointer &pointer) {
    auto it = utxoMap.find(pointer);
    if (it != utxoMap.end()) {
        UTXO utxo = it->second;
        utxoMap.erase(it);
        return utxo;
    }
    
    it = oldUTXOMap.find(pointer);
    if (it != oldUTXOMap.end()) {
        UTXO utxo = it->second;
        oldUTXOMap.erase(it);
        return utxo;
    }
    
    
    leveldb::Slice keySlice(reinterpret_cast<const char *>(&pointer), sizeof(pointer));
    std::string value;
    leveldb::Status s = levelDb->Get(leveldb::ReadOptions(), keySlice, &value);
    if (!s.ok()) {
        std::cout << "Couldn't find utxo for " << pointer.hash.GetHex() << std::endl;
        assert(false);
    }
    
    UTXO utxo = *reinterpret_cast<const UTXO *>(value.data());
    
    deletedKeys.push_back(pointer);
    return utxo;
}


void UTXOState::addOutput(UTXO utxo, const RawOutputPointer &outputPointer) {
    utxoMap.insert(std::make_pair(outputPointer, utxo));
}

void UTXOState::optionalSave() {
    if (deletedKeys.size() > DeletedKeysMaxSize) {
        clearDeletedKeys();
    }
    
    if (utxoMap.size() > (UTXOMapMaxSize * 9 / 2) / 10) {
        clearUTXOCache();
    }
}

void UTXOState::clearDeletedKeys() {
    leveldb::WriteBatch batch;
    for (auto &pointer : deletedKeys) {
        batch.Delete(leveldb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer)));
    }
    deletedKeys.clear();
    levelDb->Write(leveldb::WriteOptions(), &batch);
}

void UTXOState::clearUTXOCache() {
    if (utxoClearFuture.valid()) {
        utxoClearFuture.get();
        oldUTXOMap.clear();
        utxoMap.swap(oldUTXOMap);
    }
    
    auto utxosToSave = utxo_map(oldUTXOMap);
    utxoClearFuture = std::async(std::launch::async, [&, utxosToSave] {
        leveldb::WriteBatch batch;
        for (auto &entry : utxosToSave) {
            std::string value(reinterpret_cast<char const*>(&entry.second), sizeof(entry.second));
            batch.Put(leveldb::Slice(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first)), value);
        }
        levelDb->Write(leveldb::WriteOptions(), &batch);
    });
}
