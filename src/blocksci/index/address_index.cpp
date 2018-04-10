//
//  address_index.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_index.hpp"
#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/script_variant.hpp>

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
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_output";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_nested";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
        
        rocksdb::DB *dbPtr;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandlePtrs;
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        }
        db = std::unique_ptr<rocksdb::DB>(dbPtr);
        for (auto handle : columnHandlePtrs) {
            columnHandles.emplace_back(std::unique_ptr<rocksdb::ColumnFamilyHandle>(handle));
        }
    }
    
    const std::unique_ptr<rocksdb::ColumnFamilyHandle> &AddressIndex::getOutputColumn(AddressType::Enum type) const {
        return columnHandles[static_cast<size_t>(type)];
    }
    
    const std::unique_ptr<rocksdb::ColumnFamilyHandle> &AddressIndex::getNestedColumn(AddressType::Enum type) const {
        return columnHandles[AddressType::size + static_cast<size_t>(type)];
    }
    
    bool AddressIndex::checkIfExists(const Address &address) const {
        rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
        {
            auto it = getOutputIterator(address.type);
            it->Seek(key);
            if (it->Valid() && it->key().starts_with(key)) {
                return true;
            }
        }
        {
            auto it = getNestedIterator(address.type);
            it->Seek(key);
            if (it->Valid() && it->key().starts_with(key)) {
                return true;
            }
        }
        return false;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &address) const {
        rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
        std::vector<OutputPointer> pointers;
        auto it = getOutputIterator(address.type);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(uint32_t));
            OutputPointer outPoint;
            memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
            pointers.push_back(outPoint);
        }
        return pointers;
    }
    
    std::vector<Address> AddressIndex::getIncludingMultisigs(const Address &searchAddress) const {
        if (dedupType(searchAddress.type) != DedupAddressType::PUBKEY) {
            return {};
        }
        
        rocksdb::Slice key{reinterpret_cast<const char *>(&searchAddress.scriptNum), sizeof(searchAddress.scriptNum)};
        std::vector<Address> addresses;
        auto it = getNestedIterator(AddressType::MULTISIG_PUBKEY);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(uint32_t));
            DedupAddress rawParent;
            memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
            addresses.emplace_back(rawParent.scriptNum, AddressType::MULTISIG, searchAddress.getAccess());
        }
        return addresses;
    }
    
    std::unordered_set<Address> AddressIndex::getPossibleNestedEquivalentDown(const Address &searchAddress) const {
        std::unordered_set<Address> addressesToSearch{searchAddress};
        std::unordered_set<Address> searchedAddresses;
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            if (dedupType(a.type) == DedupAddressType::SCRIPTHASH) {
                script::ScriptHash scriptHash(a.scriptNum, searchAddress.getAccess());
                auto wrapped = scriptHash.getWrappedAddress();
                if (wrapped) {
                    for (auto type : equivAddressTypes(equivType(wrapped->type))) {
                        Address newAddress(wrapped->scriptNum, type, searchAddress.getAccess());
                        if (searchedAddresses.find(newAddress) == searchedAddresses.end()) {
                            addressesToSearch.insert(newAddress);
                        }
                    }
                }
                return true;
            }
            return false;
        };
        while (addressesToSearch.size() > 0) {
            auto setIt = addressesToSearch.begin();
            auto address = *setIt;
            visit(address, visitFunc);
            searchedAddresses.insert(address);
            addressesToSearch.erase(setIt);
        }
        
        return searchedAddresses;
    }
    
    std::unordered_set<Address> AddressIndex::getPossibleNestedEquivalentUp(const Address &searchAddress) const {
        std::unordered_set<Address> addressesToSearch{searchAddress};
        std::unordered_set<Address> searchedAddresses;
        while (addressesToSearch.size() > 0) {
            auto setIt = addressesToSearch.begin();
            auto address = *setIt;
            rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
            auto it = getNestedIterator(address.type);
            for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
                auto foundKey = it->key();
                foundKey.remove_prefix(sizeof(uint32_t));
                DedupAddress rawParent;
                memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
                switch (rawParent.type) {
                    case DedupAddressType::SCRIPTHASH:
                        for (auto type : equivAddressTypes(equivType(AddressType::SCRIPTHASH))) {
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
        
        return searchedAddresses;
    }
    
    std::vector<Address> AddressIndex::getPossibleNestedEquivalent(const Address &searchAddress) const {
        auto upAddresses = getPossibleNestedEquivalentUp(searchAddress);
        auto downAddresses = getPossibleNestedEquivalentDown(searchAddress);
        upAddresses.insert(downAddresses.begin(), downAddresses.end());
        return std::vector<Address>{upAddresses.begin(), upAddresses.end()};
    }
}

