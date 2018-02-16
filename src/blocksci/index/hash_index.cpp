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
#include "address/address.hpp"
#include "util/util.hpp"

#include <array>

namespace blocksci {
    
    HashIndex::HashIndex(const std::string &path) {
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
        
        rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        assert(s.ok());
    }
    
    HashIndex::~HashIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    template <typename T>
    uint32_t getMatch(rocksdb::DB *db, rocksdb::ColumnFamilyHandle *handle, const T &t) {
        std::string val;
        rocksdb::Slice key{reinterpret_cast<const char *>(&t), sizeof(t)};
        auto getStatus = db->Get(rocksdb::ReadOptions{}, handle, key, &val);
        if (getStatus.ok()) {
            uint32_t value;
            memcpy(&value, val.data(), sizeof(value));
            return value;
        } else {
            return 0;
        }
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) {
        return getMatch(db, columnHandles[3], txHash);
    }
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) {
        return getMatch(db, columnHandles[1], pubkeyhash);
        
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) {
        return getMatch(db, columnHandles[2], scripthash);
    }
}
