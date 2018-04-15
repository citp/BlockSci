//
//  hash_index_priv.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#ifndef hash_index_priv_hpp
#define hash_index_priv_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_info.hpp>
#include <blocksci/util/memory_view.hpp>

#include <rocksdb/db.h>

#include <range/v3/view_facade.hpp>

#include <memory>

namespace blocksci {
    class HashIndexPriv {
    public:
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;

        HashIndexPriv(const std::string &path, bool readonly);

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

        void compactDB();
    };
    
    
}

#endif /* hash_index_priv_hpp */
