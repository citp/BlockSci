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

#include <rocksdb/db.h>

#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    class DataAccess;
    class EquivAddress;
    
    class AddressIndex {
        rocksdb::DB *db;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
        
        std::unordered_set<Address> getPossibleNestedEquivalentUp(const Address &address) const;
        std::unordered_set<Address> getPossibleNestedEquivalentDown(const Address &address) const;
        std::vector<Address> getPossibleNestedEquivalent(const Address &address) const;
    public:
        
        AddressIndex(const std::string &path, bool readonly);
        ~AddressIndex();

        bool checkIfExists(const Address &address) const;
        
        std::vector<OutputPointer> getOutputPointers(const Address &address) const;
        
        EquivAddress getEquivAddresses(const Address &searchAddress, bool nestedEquivalent) const;
        EquivAddress getEquivAddresses(const DedupAddress &searchAddress, bool nestedEquivalent, const DataAccess &access) const;
        
        std::vector<Address> getIncludingMultisigs(const Address &searchAddress) const;
        
        void addAddressNested(const blocksci::Address &childAddress, const blocksci::DedupAddress &parentAddress);
        void addAddressOutput(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
        
        rocksdb::ColumnFamilyHandle *getOutputColumn(AddressType::Enum type) const;
        rocksdb::ColumnFamilyHandle *getNestedColumn(AddressType::Enum type) const;
        
        rocksdb::Iterator* getOutputIterator(AddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getOutputColumn(type));
        }
        
        rocksdb::Iterator* getNestedIterator(AddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getNestedColumn(type));
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            db->Write(rocksdb::WriteOptions(), &batch);
        }
    };
}


#endif /* address_index_hpp */
