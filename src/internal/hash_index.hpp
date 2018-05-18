//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef blocksci_index_hash_index_hpp
#define blocksci_index_hash_index_hpp

#include "address_info.hpp"
#include "memory_view.hpp"

#include <range/v3/view/any_view.hpp>

#include <rocksdb/db.h>

#include <wjfilesystem/path.h>

#include <vector>
#include <cstdint>
#include <cstring>

namespace blocksci {
    class HashIndex {
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;
        
        uint32_t lookupAddressImpl(AddressType::Enum type, const char *data, size_t size);
        void addAddressesImpl(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews);
        
        template <typename T>
        uint32_t getMatch(rocksdb::ColumnFamilyHandle *handle, const T &t) {
            rocksdb::PinnableSlice val;
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
        
        uint32_t getAddressMatch(blocksci::AddressType::Enum type, const char *data, size_t size) {
            rocksdb::PinnableSlice val;
            rocksdb::Slice key{data, size};
            auto getStatus = db->Get(rocksdb::ReadOptions{}, getColumn(type).get(), key, &val);
            if (getStatus.ok()) {
                uint32_t value;
                memcpy(&value, val.data(), sizeof(value));
                return value;
            } else {
                return 0;
            }
        }
        
        void addAddresses(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews) {
            rocksdb::WriteBatch batch;
            for (auto &pair : dataViews) {
                auto key = rocksdb::Slice(pair.first.data, pair.first.size);
                auto value = rocksdb::Slice(pair.second.data, pair.second.size);
                batch.Put(getColumn(type).get(), key, value);
            }
            writeBatch(batch);
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(AddressType::Enum type) {
            auto index = static_cast<size_t>(type);
            return columnHandles.at(index);
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(DedupAddressType::Enum type) {
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
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getTxColumn() {
            return columnHandles.back();
        }
        
        std::unique_ptr<rocksdb::Iterator> getIterator(AddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getIterator(DedupAddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getTxIterator() {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getTxColumn().get())};
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            rocksdb::WriteOptions options;
            options.disableWAL = true;
            db->Write(options, &batch);
        }
        
    public:
        
        HashIndex(const filesystem::path &path, bool readonly);
        ~HashIndex();

        template<AddressType::Enum type>
        uint32_t lookupAddress(const typename AddressInfo<type>::IDType &hash) {
            return lookupAddressImpl(type, reinterpret_cast<const char *>(&hash), sizeof(hash));
        }
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash);
        uint32_t getScriptHashIndex(const uint160 &scripthash);
        uint32_t getScriptHashIndex(const uint256 &scripthash);
        uint32_t getTxIndex(const uint256 &txHash);
        
        uint32_t countColumn(AddressType::Enum type);
        uint32_t countTxes();
        
        template<AddressType::Enum type>
        void addAddresses(std::vector<std::pair<typename blocksci::AddressInfo<type>::IDType, uint32_t>> rows) {
            std::vector<std::pair<MemoryView, MemoryView>> dataViews;
            for (const auto &pair : rows) {
                dataViews.emplace_back(
                MemoryView{reinterpret_cast<const char *>(&pair.first), sizeof(pair.first)},
                MemoryView{reinterpret_cast<const char *>(&pair.second), sizeof(pair.second)});
            }
            addAddressesImpl(type, dataViews);
        }
        
        void addTxes(std::vector<std::pair<uint256, uint32_t>> rows);
        
        void rollback(uint32_t txCount, const std::array<uint32_t, DedupAddressType::size> &scriptCounts);
        
        ranges::any_view<std::pair<MemoryView, MemoryView>> getRawAddressRange(AddressType::Enum type);
        
        template<AddressType::Enum type>
        ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<type>::IDType>> getAddressRange();
        
        void compactDB();
    };
    
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEY>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEYHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_PUBKEYHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG_PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG_PUBKEY>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::SCRIPTHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_SCRIPTHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG>();
}

#endif /* blocksci_index_hash_index_hpp */
