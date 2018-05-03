//
//  address_index_priv.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#ifndef address_index_priv_hpp
#define address_index_priv_hpp

#include <blocksci/core/address_types.hpp>

#include <rocksdb/db.h>

#include <memory>

namespace blocksci {
    class AddressIndexPriv {
    public:
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;
        
        AddressIndexPriv(const std::string &path, bool readonly);
        
        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getOutputColumn(AddressType::Enum type) const;
        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getNestedColumn(AddressType::Enum type) const;
        
        std::unique_ptr<rocksdb::Iterator> getOutputIterator(AddressType::Enum type) const {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getOutputColumn(type).get())};
        }
        
        std::unique_ptr<rocksdb::Iterator> getNestedIterator(AddressType::Enum type) const {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getNestedColumn(type).get())};
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            rocksdb::WriteOptions options;
            options.disableWAL = true;
            db->Write(options, &batch);
        }
        
        void compactDB();
    };
}

#endif /* address_index_priv_hpp */
