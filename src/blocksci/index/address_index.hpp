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
    struct Address;
    struct EquivAddress;
    
    class AddressIndex {
        rocksdb::DB *db;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    public:
        
        AddressIndex(const std::string &path, bool readonly);
        ~AddressIndex();
        
        std::vector<OutputPointer> getOutputPointers(const Address &address) const;
        std::vector<OutputPointer> getOutputPointers(const EquivAddress &script) const;
        
        std::vector<Output> getOutputs(const Address &address, const ChainAccess &access) const;
        std::vector<Input> getInputs(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const Address &address, const ChainAccess &access) const;
        
        std::vector<Output> getOutputs(const EquivAddress &script, const ChainAccess &access) const;
        std::vector<Input> getInputs(const EquivAddress &script, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const EquivAddress &script, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const EquivAddress &script, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const EquivAddress &script, const ChainAccess &access) const;
        
        void addAddressNested(const blocksci::Address &childAddress, const blocksci::EquivAddress &parentAddress);
        void addAddressOutput(const blocksci::Address &address, const blocksci::OutputPointer &pointer);
        
        rocksdb::ColumnFamilyHandle *getOutputColumn(EquivAddressType::Enum type);
        rocksdb::ColumnFamilyHandle *getNestedColumn(AddressType::Enum type);
        
        rocksdb::Iterator* getOutputIterator(EquivAddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getOutputColumn(type));
        }
        
        rocksdb::Iterator* getNestedIterator(AddressType::Enum type) {
            return db->NewIterator(rocksdb::ReadOptions(), getNestedColumn(type));
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            db->Write(rocksdb::WriteOptions(), &batch);
        }
        
        void checkDB(const ChainAccess &access) const;
    };
}


#endif /* address_index_hpp */
