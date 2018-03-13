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
#include "address/dedup_address.hpp"
#include "address/address_info.hpp"
#include "scripts/script_info.hpp"
#include "scripts/script.hpp"
#include "scripts/script_variant.hpp"

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
        blocksci::for_each(blocksci::AddressInfoList(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_output";
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
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getOutputColumn(AddressType::Enum type) const {
        return columnHandles[static_cast<size_t>(type)];
    }
    
    rocksdb::ColumnFamilyHandle *AddressIndex::getNestedColumn(AddressType::Enum type) const {
        return columnHandles[AddressType::size + static_cast<size_t>(type)];
    }
    
    AddressIndex::~AddressIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    bool AddressIndex::checkIfExists(const Address &address) const {
        auto column = getOutputColumn(address.type);
        rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        it->Seek(key);
        if (it->Valid() && it->key().starts_with(key)) {
            return true;
        }
        delete it;
        column = getNestedColumn(address.type);
        it = db->NewIterator(rocksdb::ReadOptions(), column);
        it->Seek(key);
        if (it->Valid() && it->key().starts_with(key)) {
            return true;
        }
        delete it;
        return false;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointersImp(uint32_t addressNum, AddressType::Enum type) const {
        auto column = getOutputColumn(type);
        rocksdb::Slice key{reinterpret_cast<const char *>(&addressNum), sizeof(addressNum)};
        std::vector<OutputPointer> pointers;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(uint32_t));
            OutputPointer outPoint;
            memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
            pointers.push_back(outPoint);
        }
        delete it;
        return pointers;
    }
    
    std::vector<Address> AddressIndex::getIncludingMultisigs(const Address &searchAddress) const {
        if (dedupType(searchAddress.type) != DedupAddressType::PUBKEY) {
            return {};
        }
        auto column = getNestedColumn(AddressType::MULTISIG_PUBKEY);
        rocksdb::Slice key{reinterpret_cast<const char *>(&searchAddress.scriptNum), sizeof(searchAddress.scriptNum)};
        std::vector<Address> addresses;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(uint32_t));
            DedupAddress rawParent;
            memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
            addresses.push_back(Address{rawParent.scriptNum, AddressType::MULTISIG, searchAddress.getAccess()});
        }
        delete it;
        return addresses;
    }
    
    std::vector<Address> AddressIndex::getPossibleNestedEquivalent(const Address &searchAddress) const {
        std::unordered_set<Address> addressesToSearch{searchAddress};
        std::unordered_set<Address> searchedAddresses;
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            if (dedupType(a.type) == DedupAddressType::SCRIPTHASH) {
                script::ScriptHash scriptHash(a.scriptNum, searchAddress.getAccess());
                auto wrapped = *scriptHash.getWrappedAddress();
                if (searchedAddresses.find(wrapped) == searchedAddresses.end()) {
                    addressesToSearch.insert(wrapped);
                }
                return true;
            }
            return false;
        };
        while (addressesToSearch.size() > 0) {
            auto setIt = addressesToSearch.begin();
            auto address = *setIt;
            visit(address, visitFunc);
            auto column = getNestedColumn(address.type);
            rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
            for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
                auto foundKey = it->key();
                foundKey.remove_prefix(sizeof(uint32_t));
                DedupAddress rawParent;
                memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
                switch (rawParent.type) {
                    case DedupAddressType::SCRIPTHASH:
                        for (auto type : equivAddressTypes(AddressType::SCRIPTHASH)) {
                            Address newAddress(rawParent.scriptNum, type, searchAddress.getAccess());
                            if (searchedAddresses.find(newAddress) == searchedAddresses.end()) {
                                addressesToSearch.insert(newAddress);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            searchedAddresses.insert(address);
            addressesToSearch.erase(setIt);
        }
        
        return std::vector<Address>(searchedAddresses.begin(), searchedAddresses.end());
    }
    
    std::unordered_set<Address> AddressIndex::getPossibleEquivAddresses(const Address &searchAddress, bool typeEquivalent, bool nestedEquivalent) const {
        std::unordered_set<Address> addresses;
        if (typeEquivalent && nestedEquivalent) {
            for (auto type : equivAddressTypes(searchAddress.type)) {
                auto childAddress = Address(searchAddress.scriptNum, type, searchAddress.getAccess());
                auto nested = getPossibleNestedEquivalent(childAddress);
                addresses.insert(nested.begin(), nested.end());
            }
        } else if (typeEquivalent) {
            for (auto type : equivAddressTypes(searchAddress.type)) {
                addresses.insert(Address(searchAddress.scriptNum, type, searchAddress.getAccess()));
            }
        } else if (nestedEquivalent) {
            auto nested = getPossibleNestedEquivalent(searchAddress);
            addresses.insert(nested.begin(), nested.end());
        } else {
            addresses.insert(searchAddress);
        }
        return addresses;
    }
    
    std::vector<Address> AddressIndex::getEquivAddresses(const Address &searchAddress, bool typeEquivalent, bool nestedEquivalent) const {
        std::vector<Address> addresses;
        for (const auto &address : getPossibleEquivAddresses(searchAddress, typeEquivalent, nestedEquivalent)) {
            if (checkIfExists(address)) {
                addresses.push_back(address);
            }
        }
        return addresses;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &searchAddress, bool typeEquivalent, bool nestedEquivalent) const {
        std::vector<OutputPointer> outputs;
        for (const auto &address : getPossibleEquivAddresses(searchAddress, typeEquivalent, nestedEquivalent)) {
            auto addrOuts = getOutputPointersImp(address.scriptNum, address.type);
            outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
        }
        return outputs;
    }
    
    void AddressIndex::addAddressNested(const Address &childAddress, const DedupAddress &parentAddress) {
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&childAddress.scriptNum), sizeof(childAddress.scriptNum)),
            rocksdb::Slice(reinterpret_cast<const char *>(&parentAddress), sizeof(parentAddress))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        auto nestedColumn = getNestedColumn(childAddress.type);
        db->Put(rocksdb::WriteOptions{}, nestedColumn, key, rocksdb::Slice{});
    }
    
    void AddressIndex::addAddressOutput(const Address &address, const blocksci::OutputPointer &pointer) {
        std::array<rocksdb::Slice, 2> keyParts = {{
            rocksdb::Slice(reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)),
            rocksdb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer))
        }};
        std::string sliceStr;
        rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
        auto outputColumn = getOutputColumn(address.type);
        db->Put(rocksdb::WriteOptions{}, outputColumn, key, rocksdb::Slice{});
    }
}

