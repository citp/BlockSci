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
    
    AddressIndex::AddressIndex(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        blocksci::for_each(blocksci::DedupAddressInfoList(), [&](auto tag) {
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
            rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        }
    }
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getOutputColumn(DedupAddressType::Enum type) const {
        return columnHandles[static_cast<size_t>(type)];
    }
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getNestedColumn(AddressType::Enum type) const {
        return columnHandles[DedupAddressType::size + static_cast<size_t>(type)];
    }
    
    AddressIndex::~AddressIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointersImp(const RawAddress &address) const {
        auto column = getOutputColumn(dedupType(address.type));
        rocksdb::Slice key{reinterpret_cast<const char *>(&address), sizeof(address)};
        std::vector<OutputPointer> pointers;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(RawAddress));
            OutputPointer outPoint;
            memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
            pointers.push_back(outPoint);
        }
        return pointers;
    }
    
    std::vector<RawAddress> AddressIndex::getPossibleNestedEquivalent(const RawAddress &searchAddress) const {
        std::vector<RawAddress> addresses{searchAddress};
        for (size_t i = 0; i < addresses.size(); i++) {
            auto &address = addresses[i];
            auto column = getNestedColumn(address.type);
            rocksdb::Slice key{reinterpret_cast<const char *>(&address), sizeof(address)};
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
            for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
                auto foundKey = it->key();
                foundKey.remove_prefix(sizeof(RawAddress));
                RawAddress rawParent;
                memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
                switch (rawParent.type) {
                case DedupAddressType::SCRIPTHASH:
                    for (auto type : equivAddressTypes(AddressType::SCRIPTHASH)) {
                        addresses.push_back(RawAddress(rawParent.scriptNum, type));
                    }
                    break;
                default:
                    break;
                }
            }
        }
        return addresses;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &searchAddress, bool typeEquivalent, bool nestedEquivalent) const {
        std::vector<OutputPointer> outputs;
        if (typeEquivalent && nestedEquivalent) {
            for (auto type : equivAddressTypes(searchAddress.type)) {
                auto childAddress = RawAddress(searchAddress.scriptNum, type);
                for (auto &address : getPossibleNestedEquivalent(childAddress)) {
                    auto addrOuts = getOutputPointersImp(address);
                    outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
                }
            }
        } else if (typeEquivalent) {
            for (auto type : equivAddressTypes(searchAddress.type)) {
                auto childAddress = RawAddress(searchAddress.scriptNum, type);
                auto addrOuts = getOutputPointersImp(childAddress);
                outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
            }
        } else if (nestedEquivalent) {
            for (auto &address : getPossibleNestedEquivalent(searchAddress)) {
                auto addrOuts = getOutputPointersImp(address);
                outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
            }
        } else {
            auto addrOuts = getOutputPointersImp(searchAddress);
            outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
        }
       
        return outputs;
    }
    
    void AddressIndex::addAddressNested(const Address &childAddress, const EquivAddress &parentAddress) {
        RawAddress rawChild(childAddress);
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&rawChild), sizeof(rawChild)),
            rocksdb::Slice(reinterpret_cast<const char *>(&parentAddress), sizeof(parentAddress))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        auto nestedColumn = getNestedColumn(childAddress.type);
        db->Put(rocksdb::WriteOptions{}, nestedColumn, key, rocksdb::Slice{});
    }
    
    void AddressIndex::addAddressOutput(const Address &address, const blocksci::OutputPointer &pointer) {
        RawAddress raw(address);
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&raw), sizeof(raw)),
            rocksdb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        auto outputColumn = getOutputColumn(dedupType(address.type));
        db->Put(rocksdb::WriteOptions{}, outputColumn, key, rocksdb::Slice{});
    }
}

