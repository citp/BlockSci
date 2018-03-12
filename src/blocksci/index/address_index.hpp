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

#include <unordered_map>
#include <string>
#include <vector>

namespace blocksci {
    class DataAccess;
    
    class AddressIndex {
        rocksdb::DB *db;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
        
        std::vector<OutputPointer> getOutputPointersImp(uint32_t addressNum, AddressType::Enum type) const;
    public:
        
        AddressIndex(const std::string &path, bool readonly);
        ~AddressIndex();

        std::vector<OutputPointer> getOutputPointers(const Address &searchAddress, bool typeEquivalent, bool nestedEquivalent) const;
        
        std::vector<RawAddress> getPossibleNestedEquivalent(const RawAddress &address) const;
        
        void addAddressNested(const blocksci::Address &childAddress, const blocksci::EquivAddress &parentAddress);
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
