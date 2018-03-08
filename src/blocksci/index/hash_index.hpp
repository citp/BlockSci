//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/address/address_info.hpp>

#include <rocksdb/db.h>

#include <array>
#include <vector>
#include <cstdint>


namespace blocksci {
    struct Address;
    
    class HashIndex {
    public:
        struct IndexType {
            enum Enum {
                PubkeyHash, ScriptHash, Tx
            };
            static constexpr size_t size = 3;
            static constexpr std::array<Enum, size> all = {{PubkeyHash, ScriptHash, Tx}};
            static constexpr std::array<const char*, 3> tableNames = {{"PUBKEYHASH_ADDRESS", "P2SH_ADDRESS", "TXHASH"}};
        };
        
        HashIndex(const std::string &path, bool readonly);
        ~HashIndex();
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash);
        uint32_t getScriptHashIndex(const uint160 &scripthash);
        uint32_t getScriptHashIndex(const uint256 &scripthash);
        uint32_t getTxIndex(const uint256 &txHash);
        
        template<AddressType::Enum type>
        uint32_t lookupAddress(const typename AddressInfo<type>::IDType &hash) {
            return getMatch(getColumn(type), hash);
        }
        
        template<AddressType::Enum type>
        void addAddress(const typename AddressInfo<type>::IDType &hash, uint32_t scriptNum) {
            rocksdb::Slice key(reinterpret_cast<const char *>(&hash), sizeof(hash));
            rocksdb::Slice value(reinterpret_cast<const char *>(&scriptNum), sizeof(scriptNum));
            db->Put(rocksdb::WriteOptions{}, getColumn(type), key, value);
        }
        
        uint32_t countColumn(AddressType::Enum type) {
            uint32_t keyCount = 0;
            auto column = getColumn(type);
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                keyCount++;
            }
            return keyCount;
        }
        
        void addTx(const uint256 &hash, uint32_t txID);
        
        rocksdb::Iterator* getIterator(AddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getColumn(type));
        }
        rocksdb::Iterator* getIterator(DedupAddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getColumn(type));
        }
        rocksdb::Iterator *getTxIterator() {
            return db->NewIterator(rocksdb::ReadOptions(), columnHandles.back());
        }
        
        rocksdb::ColumnFamilyHandle *getColumn(AddressType::Enum type);
        rocksdb::ColumnFamilyHandle *getColumn(DedupAddressType::Enum type);
        rocksdb::ColumnFamilyHandle *getTxColumn() {
            return columnHandles.back();
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            db->Write(rocksdb::WriteOptions(), &batch);
        }
        
        void deleteTx(const rocksdb::Slice &slice) {
            db->Delete(rocksdb::WriteOptions(), getTxColumn(), slice);
        }
        
        
    private:
        rocksdb::DB *db;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
        
        template <typename T>
        uint32_t getMatch(rocksdb::ColumnFamilyHandle *handle, const T &t) {
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
    };
}

#endif /* hash_index_hpp */
