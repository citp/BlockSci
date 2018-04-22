//
//  hash_index_priv.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#include "hash_index_priv.hpp"

#include <blocksci/meta/apply.hpp>

namespace blocksci {
    
    HashIndexPriv::HashIndexPriv(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        auto cache = rocksdb::NewLRUCache(static_cast<size_t>(1024 * 1024 * 1024));
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        for_each(AddressType::all(), [&](auto tag) {
            auto options = rocksdb::ColumnFamilyOptions{};
            auto descriptor = rocksdb::ColumnFamilyDescriptor{addressName(tag), options};
            columnDescriptors.push_back(descriptor);
        });
        auto txOptions = rocksdb::ColumnFamilyOptions{};
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions{});
        columnDescriptors.emplace_back("T", txOptions);
        
        rocksdb::DB *dbPtr;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandlePtrs;
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        }
        db = std::unique_ptr<rocksdb::DB>(dbPtr);
        for (auto handle : columnHandlePtrs) {
            columnHandles.emplace_back(std::unique_ptr<rocksdb::ColumnFamilyHandle>(handle));
        }
    }
    
    void HashIndexPriv::compactDB() {
        for (auto &column : columnHandles) {
            db->CompactRange(rocksdb::CompactRangeOptions{}, column.get(), nullptr, nullptr);
        }
    }
} // namespace blocksci

