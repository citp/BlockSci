//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include "hash_index.hpp"
#include "util/bitcoin_uint256.hpp"
#include "scripts/bitcoin_base58.hpp"
#include "scripts/script_info.hpp"
#include "address/address.hpp"
#include "util/util.hpp"

#include <array>

namespace blocksci {
    
    HashIndex::HashIndex(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
            columnDescriptors.emplace_back(scriptName(tag), rocksdb::ColumnFamilyOptions{});
        });
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions{});
        columnDescriptors.emplace_back("T", rocksdb::ColumnFamilyOptions{});
        
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandles, &db);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        }
    }
    
    HashIndex::~HashIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    rocksdb::ColumnFamilyHandle *HashIndex::getColumn(AddressType::Enum type) {
        auto index = static_cast<size_t>(type);
        if (index > AddressType::size) {
            assert("Tried to get column for unindexed script type");
            return nullptr;
        }
        return columnHandles[index];
    }
    
    rocksdb::ColumnFamilyHandle *HashIndex::getColumn(ScriptType::Enum type) {
        switch (type) {
            case ScriptType::PUBKEY:
                return getColumn(AddressType::PUBKEY);
            case ScriptType::SCRIPTHASH:
                return getColumn(AddressType::SCRIPTHASH);
            case ScriptType::MULTISIG:
                return getColumn(AddressType::MULTISIG);
            case ScriptType::NULL_DATA:
                return getColumn(AddressType::NULL_DATA);
            case ScriptType::NONSTANDARD:
                return getColumn(AddressType::NONSTANDARD);
        }
        assert(false);
        return getColumn(AddressType::PUBKEY);
    }
    
    void HashIndex::addTx(const uint256 &hash, uint32_t txNum) {
        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&hash), sizeof(hash));
        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&txNum), sizeof(txNum));
        db->Put(rocksdb::WriteOptions(), columnHandles.back(), keySlice, valueSlice);
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) {
        return getMatch(columnHandles.back(), txHash);
    }
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) {
        return getMatch(getColumn(AddressType::PUBKEY), pubkeyhash);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) {
        return getMatch(getColumn(AddressType::SCRIPTHASH), scripthash);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint256 &scripthash) {
        return getMatch(getColumn(AddressType::SCRIPTHASH), scripthash);
    }
}
