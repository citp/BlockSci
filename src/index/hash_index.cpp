//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include <blocksci/index/hash_index.hpp>
#include "hash_index_priv.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/meta.hpp>
#include <blocksci/util/state.hpp>

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
    
    HashIndex::HashIndex(const std::string &path, bool readonly) : impl(std::make_unique<HashIndexPriv>(path, readonly)) {}
    
    HashIndex::~HashIndex() = default;
    
    void HashIndex::addTxes(std::vector<std::pair<uint256, uint32_t>> rows) {
        rocksdb::WriteBatch batch;
        for (const auto &pair : rows) {
            rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
            rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
            batch.Put(impl->getTxColumn().get(), keySlice, valueSlice);
        }
        impl->writeBatch(batch);
    }
    
    uint32_t HashIndex::countColumn(AddressType::Enum type) {
        uint32_t keyCount = 0;
        auto it = impl->getIterator(type);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            keyCount++;
        }
        return keyCount;
    }
    
    uint32_t HashIndex::countTxes() {
        uint32_t keyCount = 0;
        auto it = impl->getTxIterator();
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            keyCount++;
        }
        return keyCount;
    }
    
    ColumnIterator HashIndex::getRawAddressRange(AddressType::Enum type) {
        return ColumnIterator(impl->db.get(), impl->getColumn(type).get());
    }
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) {
        return lookupAddress<AddressType::PUBKEYHASH>(pubkeyhash);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) {
        return lookupAddress<AddressType::SCRIPTHASH>(scripthash);
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint256 &scripthash) {
        return lookupAddress<AddressType::WITNESS_SCRIPTHASH>(scripthash);
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) {
        return impl->getMatch(impl->getTxColumn().get(), txHash);
    }
    
    void HashIndex::addAddressesImpl(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews) {
        impl->addAddresses(type, dataViews);
    }
    
    uint32_t HashIndex::lookupAddressImpl(blocksci::AddressType::Enum type, const char *data, size_t size) {
        return impl->getAddressMatch(type, data, size);
    }
    
    void HashIndex::compactDB() {
        impl->compactDB();
    }
    
    void HashIndex::rollback(const State &state) {
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            auto &column = impl->getColumn(tag);
            rocksdb::WriteBatch batch;
            auto it = impl->getIterator(tag);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                uint32_t destNum;
                memcpy(&destNum, it->value().data(), sizeof(destNum));
                auto count = state.scriptCounts[static_cast<size_t>(tag)];
                if (destNum >= count) {
                    batch.Delete(column.get(), it->key());
                }
            }
            assert(it->status().ok());
            impl->writeBatch(batch);
        });
        {
            auto it = impl->getTxIterator();
            rocksdb::WriteBatch batch;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                uint32_t value;
                memcpy(&value, it->value().data(), sizeof(value));
                if (value >= state.txCount) {
                    batch.Delete(impl->getTxColumn().get(), it->key());
                }
            }
            impl->writeBatch(batch);
            assert(it->status().ok()); // Check for any errors found during the scan
        }
    }
}
