//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include "hash_index.hpp"
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/scripts/bitcoin_base58.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/util/util.hpp>

#include <rocksdb/table.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/slice_transform.h>
#include <rocksdb/table_properties.h>

#include <array>
//
//namespace {
//    void OptimizeForPointLookup(rocksdb::ColumnFamilyOptions &options, std::shared_ptr<rocksdb::Cache> cache) {
//        options.prefix_extractor.reset(rocksdb::NewNoopTransform());
//        rocksdb::BlockBasedTableOptions block_based_options;
//        block_based_options.index_type = rocksdb::BlockBasedTableOptions::kHashSearch;
//        block_based_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
//        block_based_options.block_cache = std::move(cache);
//        options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(block_based_options));
//        options.memtable_prefix_bloom_size_ratio = 0.02;
//    }
//};

namespace blocksci {
    
    HashIndex::HashIndex(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        auto cache = rocksdb::NewLRUCache(static_cast<size_t>(1024 * 1024 * 1024));
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            auto options = rocksdb::ColumnFamilyOptions{};
            auto descriptor = rocksdb::ColumnFamilyDescriptor{addressName(tag), options};
            columnDescriptors.push_back(descriptor);
        });
        auto txOptions = rocksdb::ColumnFamilyOptions{};
        columnDescriptors.emplace_back("T", txOptions);
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions{});
        
        rocksdb::DB *dbPtr;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandlePtrs;
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
        }
        db = std::unique_ptr<rocksdb::DB>(dbPtr);
        for (auto handle : columnHandlePtrs) {
            columnHandles.emplace_back(std::unique_ptr<rocksdb::ColumnFamilyHandle>(handle));
        }
    }
    
    std::unique_ptr<rocksdb::ColumnFamilyHandle> &HashIndex::getColumn(AddressType::Enum type) {
        auto index = static_cast<size_t>(type);
        return columnHandles.at(index);
    }
    
    std::unique_ptr<rocksdb::ColumnFamilyHandle> &HashIndex::getColumn(DedupAddressType::Enum type) {
        switch (type) {
            case DedupAddressType::PUBKEY:
                return getColumn(AddressType::PUBKEYHASH);
            case DedupAddressType::SCRIPTHASH:
                return getColumn(AddressType::SCRIPTHASH);
            case DedupAddressType::MULTISIG:
                return getColumn(AddressType::MULTISIG);
            case DedupAddressType::NULL_DATA:
                return getColumn(AddressType::NULL_DATA);
            case DedupAddressType::NONSTANDARD:
                return getColumn(AddressType::NONSTANDARD);
        }
        assert(false);
        return getColumn(AddressType::NONSTANDARD);
    }
}
