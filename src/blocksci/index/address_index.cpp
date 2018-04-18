//
//  address_index.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_index.hpp"
#include "address_index_priv.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/util/state.hpp>

#include <range/v3/utility/optional.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/to_container.hpp>

#include <unordered_set>
#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    AddressIndex::AddressIndex(const std::string &path, bool readonly) : impl(std::make_unique<AddressIndexPriv>(path, readonly)) {}
    
    AddressIndex::~AddressIndex() = default;
    
    void AddressIndex::compactDB() {
        impl->compactDB();
    }
    
    void AddressIndex::addNestedAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::DedupAddress>> nestedCache) {
        rocksdb::WriteBatch batch;
        for (auto &pair : nestedCache) {
            const RawAddress &childAddress = pair.first;
            const DedupAddress &parentAddress = pair.second;
            std::array<rocksdb::Slice, 2> keyParts = {{
                rocksdb::Slice(reinterpret_cast<const char *>(&childAddress.scriptNum), sizeof(childAddress.scriptNum)),
                rocksdb::Slice(reinterpret_cast<const char *>(&parentAddress), sizeof(parentAddress))
            }};
            std::string sliceStr;
            rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
            auto &nestedColumn = impl->getNestedColumn(childAddress.type);
            batch.Put(nestedColumn.get(), key, rocksdb::Slice{});
        }
        impl->writeBatch(batch);
    }
    
    void AddressIndex::addOutputAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::OutputPointer>> outputCache) {
        rocksdb::WriteBatch batch;
        for (auto &pair : outputCache) {
            const RawAddress &address = pair.first;
            const blocksci::OutputPointer &pointer = pair.second;
            std::array<rocksdb::Slice, 2> keyParts = {{
                rocksdb::Slice(reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)),
                rocksdb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer))
            }};
            std::string sliceStr;
            rocksdb::Slice key{rocksdb::SliceParts{keyParts.data(), keyParts.size()}, &sliceStr};
            auto &outputColumn = impl->getOutputColumn(address.type);
            batch.Put(outputColumn.get(), key, rocksdb::Slice{});
        }
        impl->writeBatch(batch);
    }
    
    void AddressIndex::rollback(const State &state) {
        for_each(AddressType::all(), [&](auto type) {
            auto &column = impl->getOutputColumn(type);
            auto it = impl->getOutputIterator(type);
            rocksdb::WriteBatch batch;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                auto key = it->key();
                key.remove_prefix(sizeof(uint32_t));
                OutputPointer outPoint;
                memcpy(&outPoint, key.data(), sizeof(outPoint));
                if (outPoint.txNum >= state.scriptCounts[static_cast<size_t>(blocksci::DedupAddressType::SCRIPTHASH)]) {
                    batch.Delete(column.get(), it->key());
                }
            }
            assert(it->status().ok()); // Check for any errors found during the scan
        });
    }
    
    bool AddressIndex::checkIfExists(const Address &address) const {
        rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
        {
            auto it = impl->getOutputIterator(address.type);
            it->Seek(key);
            if (it->Valid() && it->key().starts_with(key)) {
                return true;
            }
        }
        {
            auto it = impl->getNestedIterator(address.type);
            it->Seek(key);
            if (it->Valid() && it->key().starts_with(key)) {
                return true;
            }
        }
        return false;
    }
    
    bool AddressIndex::checkIfTopLevel(const Address &address) const {
        rocksdb::Slice key{reinterpret_cast<const char *>(&address.scriptNum), sizeof(address.scriptNum)};
        auto it = impl->getOutputIterator(address.type);
        it->Seek(key);
        if (it->Valid() && it->key().starts_with(key)) {
            return true;
        }
        return false;
    }
    
    ColumnIterator AddressIndex::getRawOutputPointerRange(const Address &address) const {
        auto prefixData = reinterpret_cast<const char *>(&address.scriptNum);
        std::vector<char> prefix(prefixData, prefixData + sizeof(address.scriptNum));
        return ColumnIterator(impl->db.get(), impl->getOutputColumn(address.type).get(), prefix);
    }
    
    std::vector<Address> AddressIndex::getIncludingMultisigs(const Address &searchAddress) const {
        if (dedupType(searchAddress.type) != DedupAddressType::PUBKEY) {
            return {};
        }
        
        rocksdb::Slice key{reinterpret_cast<const char *>(&searchAddress.scriptNum), sizeof(searchAddress.scriptNum)};
        std::vector<Address> addresses;
        auto it = impl->getNestedIterator(AddressType::MULTISIG_PUBKEY);
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
            auto it = impl->getNestedIterator(address.type);
            for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
                auto foundKey = it->key();
                foundKey.remove_prefix(sizeof(uint32_t));
                DedupAddress rawParent;
                memcpy(&rawParent, foundKey.data(), sizeof(rawParent));
                if (rawParent.type == DedupAddressType::SCRIPTHASH) {
                    for (auto type : equivAddressTypes(equivType(AddressType::SCRIPTHASH))) {
                        Address newAddress(rawParent.scriptNum, type, searchAddress.getAccess());
                        if (searchedAddresses.find(newAddress) == searchedAddresses.end()) {
                            addressesToSearch.insert(newAddress);
                        }
                    }
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
    
    AddressOutputRange::cursor::cursor() : access(nullptr), it(nullptr) {}
    
    AddressOutputRange::cursor::cursor(DataAccess &access_) : access(&access_), it(nullptr) {
        advanceToNext();
    }
    
    AddressOutputRange::cursor::cursor(const cursor &other) : access(other.access), rowNum(other.rowNum), currentTypeIndex(other.currentTypeIndex) {
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
    }
    
    AddressOutputRange::cursor::~cursor() = default;
    
    AddressOutputRange::cursor &AddressOutputRange::cursor::operator=(const cursor &other) {
        access = other.access;
        rowNum = other.rowNum;
        currentTypeIndex = other.currentTypeIndex;
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
        return *this;
    }
    
    std::pair<Address, OutputPointer> AddressOutputRange::cursor::read() const {
        auto key = it->key();
        uint32_t addressNum;
        OutputPointer outPoint;
        memcpy(&addressNum, key.data(), sizeof(addressNum));
        key.remove_prefix(sizeof(addressNum));
        memcpy(&outPoint, key.data(), sizeof(outPoint));
        return std::make_pair(Address{addressNum, static_cast<AddressType::Enum>(currentTypeIndex), *access}, outPoint);
    }
    
    void AddressOutputRange::cursor::advanceToNext() {
        while (it == nullptr || !it->Valid()) {
            currentTypeIndex++;
            if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToFirst();
            } else {
                it.reset(nullptr);
                break;
            }
        }
    }
    
    bool AddressOutputRange::cursor::equal(ranges::default_sentinel) const {
        return !it->Valid() && currentTypeIndex == AddressType::size;
    }
    
    void AddressOutputRange::cursor::next() {
        it->Next();
        rowNum++;
        if (!it->Valid()) {
            advanceToNext();
        }
    }
    
    void AddressOutputRange::cursor::prev() {
        it->Prev();
        rowNum--;
        if (!it->Valid()) {
            if (currentTypeIndex > 0) {
                currentTypeIndex--;
                it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToLast();
            } else {
                it.reset(nullptr);
            }
        }
    }
}

