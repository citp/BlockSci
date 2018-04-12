//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_info.hpp>

#include <rocksdb/db.h>

#include <array>
#include <vector>
#include <cstdint>


namespace blocksci {
    class BLOCKSCI_EXPORT HashIndex {
    public:
        HashIndex(const std::string &path, bool readonly);
        
        template<AddressType::Enum type>
        uint32_t lookupAddress(const typename AddressInfo<type>::IDType &hash) {
            return getMatch(getColumn(type).get(), hash);
        }
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash) {
            return lookupAddress<AddressType::PUBKEYHASH>(pubkeyhash);
        }
        
        uint32_t getScriptHashIndex(const uint160 &scripthash) {
            return lookupAddress<AddressType::SCRIPTHASH>(scripthash);
        }
        
        uint32_t getScriptHashIndex(const uint256 &scripthash) {
            return lookupAddress<AddressType::WITNESS_SCRIPTHASH>(scripthash);
        }
        uint32_t getTxIndex(const uint256 &txHash) {
            return getMatch(columnHandles.back().get(), txHash);
        }
        
        uint32_t countColumn(AddressType::Enum type) {
            uint32_t keyCount = 0;
            auto it = getIterator(type);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                keyCount++;
            }
            return keyCount;
        }
        
        std::unique_ptr<rocksdb::Iterator> getIterator(AddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getIterator(DedupAddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getTxIterator() {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), columnHandles.back().get())};
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(AddressType::Enum type);
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(DedupAddressType::Enum type);
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getTxColumn() {
            return columnHandles.back();
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            rocksdb::WriteOptions options;
            options.disableWAL = true;
            db->Write(options, &batch);
        }
        
        void compactDB() {
            for (auto &column : columnHandles) {
                db->CompactRange(rocksdb::CompactRangeOptions{}, column.get(), nullptr, nullptr);
            }
        }
        
    private:
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;
        
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
    };
}

#endif /* hash_index_hpp */
