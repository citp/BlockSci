//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <blocksci/core/address_types.hpp>
#include <blocksci/core/core_fwd.hpp>

#include <range/v3/view/any_view.hpp>

#include <rocksdb/db.h>

#include <wjfilesystem/path.h>

#include <memory>
#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    struct DedupAddress;
    class RawAddressOutputRange;
    
    class AddressIndex {
        friend class RawAddressOutputRange;
        
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;

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
        
    public:
        
        AddressIndex(const filesystem::path &path, bool readonly);
        ~AddressIndex();
        
        ranges::any_view<InoutPointer, ranges::category::forward> getOutputPointers(const RawAddress &address) const;
        
        ranges::optional<DedupAddress> getNestingScriptHash(const RawAddress &searchAddress) const;
        std::unordered_set<DedupAddress> getPossibleNestedEquivalentUp(const RawAddress &searchAddress) const;
        ranges::any_view<RawAddress> getIncludingMultisigs(const RawAddress &searchAddress) const;
        
        void addNestedAddresses(std::vector<std::pair<RawAddress, DedupAddress>> nestedCache);
        void addOutputAddresses(std::vector<std::pair<RawAddress, InoutPointer>> outputCache);
        
        void compactDB();
        
        void rollback(uint32_t txNum);
    };
}


#endif /* address_index_hpp */
