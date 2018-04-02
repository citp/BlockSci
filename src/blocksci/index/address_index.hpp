//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/address/address_info.hpp>

#include <rocksdb/db.h>

#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    class DataAccess;
    class EquivAddress;
    
    class AddressIndex {
        std::unique_ptr<rocksdb::DB> db;
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;
        
        std::unordered_set<Address> getPossibleNestedEquivalentUp(const Address &address) const;
        std::unordered_set<Address> getPossibleNestedEquivalentDown(const Address &address) const;
        
    public:
        
        AddressIndex(const std::string &path, bool readonly);

        bool checkIfExists(const Address &address) const;
        std::vector<OutputPointer> getOutputPointers(const Address &address) const;
        std::vector<Address> getPossibleNestedEquivalent(const Address &address) const;
        std::vector<Address> getIncludingMultisigs(const Address &searchAddress) const;
        
        void addAddressNested(const blocksci::Address &childAddress, const blocksci::DedupAddress &parentAddress);
        void addAddressOutput(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
        
        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getOutputColumn(AddressType::Enum type) const;
        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getNestedColumn(AddressType::Enum type) const;
        
       std::unique_ptr<rocksdb::Iterator> getOutputIterator(AddressType::Enum type) const {
           return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getOutputColumn(type).get())};
        }
        
        std::unique_ptr<rocksdb::Iterator> getNestedIterator(AddressType::Enum type) const {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getNestedColumn(type).get())};
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            db->Write(rocksdb::WriteOptions(), &batch);
        }
        
        void compactDB() {
            for (auto &column : columnHandles) {
                db->CompactRange(rocksdb::CompactRangeOptions{}, column.get(), nullptr, nullptr);
            }
        }
    };
}


#endif /* address_index_hpp */
