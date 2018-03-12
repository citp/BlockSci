//
//  address_index.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_index.hpp"
#include "chain/transaction.hpp"
#include "chain/inout_pointer.hpp"
#include "chain/output.hpp"
#include "chain/input.hpp"
#include "address/address.hpp"
#include "address/equiv_address.hpp"
#include "address/address_info.hpp"
#include "scripts/script_info.hpp"
#include "scripts/script.hpp"

#include <range/v3/utility/optional.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/to_container.hpp>

#include <unordered_set>
#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    std::vector<Output> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Input> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    
    AddressIndex::AddressIndex(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        blocksci::for_each(blocksci::EquivAddressInfoList(), [&](auto tag) {
            std::stringstream ss;
            ss << equivAddressName(tag) << "_output";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        blocksci::for_each(blocksci::AddressInfoList(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_nested";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
        
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandles, &db);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        }
    }
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getOutputColumn(EquivAddressType::Enum type) {
        return columnHandles[static_cast<size_t>(type)];
    }
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getNestedColumn(AddressType::Enum type) {
        return columnHandles[EquivAddressType::size + static_cast<size_t>(type)];
    }
    
    AddressIndex::~AddressIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    std::vector<OutputPointer> getOutputPointersImp(rocksdb::DB *db, rocksdb::ColumnFamilyHandle *column, const rocksdb::Slice &key) {
        std::vector<OutputPointer> pointers;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(Address));
            OutputPointer outPoint;
            memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
            pointers.push_back(outPoint);
        }
        return pointers;
    }
                                                                      
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &searchAddress) const {
        auto column = columnHandles[static_cast<size_t>(equivType(searchAddress.type)) + 1];
        rocksdb::Slice key{reinterpret_cast<const char *>(&searchAddress), sizeof(searchAddress)};
        return getOutputPointersImp(db, column, key);
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const EquivAddress &script) const {
        auto column = columnHandles[static_cast<size_t>(script.type) + 1];
        rocksdb::Slice key{reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum)};
        return getOutputPointersImp(db, column, key);
    }
    
    void AddressIndex::addAddressNested(const Address &childAddress, const EquivAddress &parentAddress) {
        auto nestedColumn = getNestedColumn(childAddress.type);
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&childAddress), sizeof(childAddress)),
            rocksdb::Slice(reinterpret_cast<const char *>(&parentAddress), sizeof(parentAddress))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        db->Put(rocksdb::WriteOptions{}, nestedColumn, key, rocksdb::Slice{});
    }
    
    void AddressIndex::addAddressOutput(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
        auto script = equivType(address.type);
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&address), sizeof(address)),
            rocksdb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        db->Put(rocksdb::WriteOptions{}, columnHandles[static_cast<size_t>(script) + 1], key, rocksdb::Slice{});
    }
    
    void AddressIndex::checkDB(const DataAccess &access) const {
        for (auto scriptType : EquivAddressType::all) {
            auto column = columnHandles[static_cast<size_t>(scriptType) + 1];
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                auto foundKey = it->key();
                Address address;
                memcpy(&address, foundKey.data(), sizeof(address));
                foundKey.remove_prefix(sizeof(Address));
                OutputPointer outPoint;
                memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
                Output output(outPoint, access);
                if (output.getType() != address.type) {
                    std::cout << "Output " << output << " matched with " << address << " instead of " << output.getAddress();
                    continue;
                }
            }
        }
    }
}

