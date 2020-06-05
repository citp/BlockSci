//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include "hash_index.hpp"
#include "column_iterator.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>

#include <range/v3/view/transform.hpp>

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
    
    HashIndex::HashIndex(const filesystem::path &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();

        // Create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        auto cache = rocksdb::NewLRUCache(static_cast<size_t>(1024 * 1024 * 1024));

        // Initialize RocksDB column families
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        columnDescriptors.reserve(AddressType::size + 2);
        for_each(AddressType::all(), [&](auto tag) {
            columnDescriptors.emplace_back(addressName(tag), rocksdb::ColumnFamilyOptions{});
        });
        auto txOptions = rocksdb::ColumnFamilyOptions{};
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions{});
        columnDescriptors.emplace_back("T", txOptions);
        
        rocksdb::DB *dbPtr;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandlePtrs;
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.str().c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            if (!s.ok()) {
                throw std::runtime_error{"Could not open hash index with error: " + std::string{s.getState()}};
            }
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.str().c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            if (!s.ok()) {
                throw std::runtime_error{"Could not open hash index with error: " + std::string{s.getState()}};
            }
        }
        db = std::unique_ptr<rocksdb::DB>(dbPtr);
        for (auto handle : columnHandlePtrs) {
            columnHandles.emplace_back(std::unique_ptr<rocksdb::ColumnFamilyHandle>(handle));
        }
    }
    
    HashIndex::~HashIndex() = default;
    
    void HashIndex::compactDB() {
        for (auto &column : columnHandles) {
            db->CompactRange(rocksdb::CompactRangeOptions{}, column.get(), nullptr, nullptr);
        }
    }
    
    void HashIndex::addTxes(std::vector<std::pair<uint256, uint32_t>> rows) {
        rocksdb::WriteBatch batch;
        for (const auto &pair : rows) {
            rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
            rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
            batch.Put(getTxColumn().get(), keySlice, valueSlice);
        }
        writeBatch(batch);
    }
    
    uint32_t HashIndex::countColumn(AddressType::Enum type) {
        uint32_t keyCount = 0;
        auto it = getIterator(type);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            keyCount++;
        }
        return keyCount;
    }
    
    uint32_t HashIndex::countTxes() {
        uint32_t keyCount = 0;
        auto it = getTxIterator();
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            keyCount++;
        }
        return keyCount;
    }
    
    ranges::any_view<std::pair<MemoryView, MemoryView>> HashIndex::getRawAddressRange(AddressType::Enum type) {
        return ColumnIterator(db.get(), getColumn(type).get());
    }
    
    ranges::optional<uint32_t> HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) {
        return lookupAddress<AddressType::PUBKEYHASH>(pubkeyhash);
    }
    
    ranges::optional<uint32_t> HashIndex::getScriptHashIndex(const uint160 &scripthash) {
        return lookupAddress<AddressType::SCRIPTHASH>(scripthash);
    }
    
    ranges::optional<uint32_t> HashIndex::getScriptHashIndex(const uint256 &scripthash) {
        return lookupAddress<AddressType::WITNESS_SCRIPTHASH>(scripthash);
    }
    
    ranges::optional<uint32_t> HashIndex::getTxIndex(const uint256 &txHash) {
        return getMatch(getTxColumn().get(), txHash);
    }
    
    ranges::optional<uint32_t> HashIndex::lookupAddressImpl(blocksci::AddressType::Enum type, const char *data, size_t size) {
        return getAddressMatch(type, data, size);
    }

    void HashIndex::addAddressesImpl(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews) {
        addAddresses(type, dataViews);
    }
    
    void HashIndex::rollback(uint32_t txCount, const std::array<uint32_t, DedupAddressType::size> &scriptCounts) {
        // foreach over all ADDRESS_TYPE_LIST items (= a list of integers from AddressType::Enum)
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            auto &column = getColumn(tag);
            rocksdb::WriteBatch batch;
            auto it = getIterator(tag);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                uint32_t destNum;
                memcpy(&destNum, it->value().data(), sizeof(destNum));
                auto count = scriptCounts[static_cast<size_t>(tag)];
                if (destNum >= count) {
                    batch.Delete(column.get(), it->key());
                }
            }
            assert(it->status().ok());
            writeBatch(batch);
        });

        // Delete all entries with value higher than txCount
        {
            auto it = getTxIterator();
            rocksdb::WriteBatch batch;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                uint32_t value;
                memcpy(&value, it->value().data(), sizeof(value));
                if (value >= txCount) {
                    batch.Delete(getTxColumn().get(), it->key());
                }
            }
            writeBatch(batch);
            assert(it->status().ok());  // Check for errors found during the scan
        }
    }
    
    template<AddressType::Enum type>
    ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<type>::IDType>> HashIndex::getAddressRange() {
        using IDType = typename blocksci::AddressInfo<type>::IDType;
        return getRawAddressRange(type) | ranges::views::transform([](std::pair<MemoryView, MemoryView> pair) -> std::pair<uint32_t, IDType> {
            auto &key = pair.first;
            auto &value = pair.second;
            uint32_t addressNum;
            IDType hash;
            memcpy(&addressNum, value.data, sizeof(addressNum));
            memcpy(&hash, key.data, sizeof(hash));
            return {addressNum, hash};
        });
    }
    
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEY>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEYHASH>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_PUBKEYHASH>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG_PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG_PUBKEY>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::SCRIPTHASH>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_SCRIPTHASH>();
    template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG>();
}
