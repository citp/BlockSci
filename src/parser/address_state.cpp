//
//  address_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "address_state.hpp"
#include "parser_configuration.hpp"

#include <hyperleveldb/db.h>
#include <hyperleveldb/write_batch.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string_view>

namespace {
    using namespace std::string_view_literals;
    
    static constexpr auto singleAddressFileName = "single.dat"sv;
    static constexpr auto multiAddressFileName = "multi.dat"sv;
    static constexpr auto dbFileName = "db"sv;
    static constexpr auto bloomFileName = "bloom"sv;
    static constexpr auto scriptCountsFileName = "scriptCounts.txt"sv;
}

AddressState::AddressState(const boost::filesystem::path &path_) : path(path_), addressBloomFilter(path/std::string(bloomFileName), StartingAddressCount, AddressFalsePositiveRate)  {
    leveldb::Options options;
    options.create_if_missing = true;
    options.write_buffer_size = 128 * 1024* 1024;
    leveldb::DB::Open(options, (path/std::string(dbFileName)).c_str(), &levelDb);
    
    blocksci::uint160 deletedAddress;
    deletedAddress.SetHex("FFFFFFFFFFFFFFFFFFFF");
    blocksci::RawScript deletedKey{deletedAddress, blocksci::ScriptType::Enum::NULL_DATA};
    
    singleAddressMap.set_deleted_key(deletedKey);
    oldSingleAddressMap.set_deleted_key(deletedKey);
    multiAddressMap.set_deleted_key(deletedKey);
    
    blocksci::RawScript emptyKey{deletedAddress, blocksci::ScriptType::Enum::SCRIPTHASH};
    
    singleAddressMap.set_empty_key(emptyKey);
    oldSingleAddressMap.set_empty_key(emptyKey);
    multiAddressMap.set_empty_key(emptyKey);

    singleAddressMap.resize(SingleAddressMapMaxSize);
    oldSingleAddressMap.resize(SingleAddressMapMaxSize);
    multiAddressMap.resize(SingleAddressMapMaxSize);
    
    if (auto file = fopen((path/std::string(singleAddressFileName)).c_str(), "rb"); file != NULL) {
        singleAddressMap.unserialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    
    if (auto file = fopen((path/std::string(multiAddressFileName)).c_str(), "rb"); file != NULL) {
        multiAddressMap.unserialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    
    boost::filesystem::ifstream inputFile(path/std::string(scriptCountsFileName));
    
    if (inputFile) {
        uint32_t value;
        while ( inputFile >> value ) {
            scriptIndexes.push_back(value);
        }
    } else {
        for (size_t i = 0; i < blocksci::ScriptType::all.size(); i++) {
            scriptIndexes.push_back(1);
        }
    }
}

AddressState::~AddressState() {
    if (addressClearFuture.valid()) {
        addressClearFuture.get();
    }
    
    if (auto file = fopen((path/std::string(singleAddressFileName)).c_str(), "wb"); file != NULL) {
        singleAddressMap.serialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    if (auto file = fopen((path/std::string(multiAddressFileName)).c_str(), "wb"); file != NULL) {
        multiAddressMap.serialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    
    boost::filesystem::ofstream outputFile(path/std::string(scriptCountsFileName));
    for (auto value : scriptIndexes) {
        outputFile << value << " ";
    }
}

uint32_t AddressState::getNewAddressIndex(blocksci::ScriptType::Enum type) {
    auto &count = scriptIndexes[static_cast<uint8_t>(type)];
    auto scriptNum = count;
    count++;
    return scriptNum;
}

void AddressState::reloadBloomFilter() {
    for (auto &pair : singleAddressMap) {
        addressBloomFilter.add(pair.first);
    }
    
    for (auto &pair : oldSingleAddressMap) {
        addressBloomFilter.add(pair.first);
    }
    
    for (auto &pair : multiAddressMap) {
        addressBloomFilter.add(pair.first);
    }
    
    leveldb::Iterator* it = levelDb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        if (key.size() == sizeof(blocksci::RawScript)) {
            auto address = *reinterpret_cast<const blocksci::RawScript *>(key.data());
            addressBloomFilter.add(address);
        }
    }
}

std::pair<uint32_t, bool> AddressState::resolveAddress(const AddressInfo &addressInfo) {
    auto rawAddress = addressInfo.rawAddress;
    bool existingAddress = false;
    switch (addressInfo.location) {
        case AddressLocation::SingleUseMap:
            singleAddressMap.erase(addressInfo.it);
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            existingAddress = true;
            break;
        case AddressLocation::OldSingleUseMap:
            oldSingleAddressMap.erase(addressInfo.it);
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            existingAddress = true;
            break;
        case AddressLocation::LevelDb:
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            existingAddress = true;
            break;
        case AddressLocation::MultiUseMap:
            existingAddress = true;
            break;
        case AddressLocation::NotFound:
            existingAddress = false;
            break;
    }
    
    uint32_t addressNum = addressInfo.addressNum;
    if (!existingAddress) {
        addressNum = getNewAddressIndex(rawAddress.type);
        addressBloomFilter.add(rawAddress);
        singleAddressMap.insert(std::make_pair(rawAddress, addressNum));
        
        if (addressBloomFilter.isFull()) {
            addressBloomFilter.reset(addressBloomFilter.getMaxItems() * 2, addressBloomFilter.getFPRate());
            reloadBloomFilter();
        }
    }
    return std::make_pair(addressNum, !existingAddress);
}

void AddressState::rollback(const blocksci::State &state) {
    for (auto multiAddressIt = multiAddressMap.begin(); multiAddressIt != multiAddressMap.end(); ++multiAddressIt) {
        auto count = state.scriptCounts[static_cast<size_t>(multiAddressIt->first.type)];
        if (multiAddressIt->second >= count) {
            multiAddressMap.erase(multiAddressIt);
        }
    }
    
    for (auto singleAddressIt = singleAddressMap.begin(); singleAddressIt != singleAddressMap.end(); ++singleAddressIt) {
        auto count = state.scriptCounts[static_cast<size_t>(singleAddressIt->first.type)];
        if (singleAddressIt->second >= count) {
            singleAddressMap.erase(singleAddressIt);
        }
    }
    
    for (auto singleAddressIt = oldSingleAddressMap.begin(); singleAddressIt != oldSingleAddressMap.end(); ++singleAddressIt) {
        auto count = state.scriptCounts[static_cast<size_t>(singleAddressIt->first.type)];
        if (singleAddressIt->second >= count) {
            oldSingleAddressMap.erase(singleAddressIt);
        }
    }
    
    leveldb::WriteBatch batch;
    leveldb::Iterator* levelDbIt = levelDb->NewIterator(leveldb::ReadOptions());
    for (levelDbIt->SeekToFirst(); levelDbIt->Valid(); levelDbIt->Next()) {
        auto key = levelDbIt->key();
        if (key.size() == sizeof(blocksci::RawScript)) {
            uint32_t destNum = *reinterpret_cast<const uint32_t *>(levelDbIt->value().data());
            auto address = *reinterpret_cast<const blocksci::RawScript *>(key.data());
            auto count = state.scriptCounts[static_cast<size_t>(address.type)];
            if (destNum >= count) {
                batch.Delete(key);
            }
        }
    }
    levelDb->Write(leveldb::WriteOptions(), &batch);
    addressBloomFilter.reset(addressBloomFilter.getMaxItems(), addressBloomFilter.getFPRate());
    reloadBloomFilter();
    scriptIndexes.clear();
    for (auto size : state.scriptCounts) {
        scriptIndexes.push_back(size);
    }
}

AddressInfo AddressState::findAddress(const blocksci::RawScript &address) const {
    static uint64_t fpcount = 0;
    static uint64_t tpcount = 0;
    static uint64_t ldbcount = 0;
    
    auto &_singleAddressMap = const_cast<address_map &>(singleAddressMap);
    auto &_oldSingleAddressMap = const_cast<address_map &>(oldSingleAddressMap);
    auto &_multiAddressMap = const_cast<address_map &>(multiAddressMap);
    
    if (!addressBloomFilter.possiblyContains(address)) {
        // Address has definitely never been seen
        tpcount++;
        return {address, AddressLocation::NotFound, _singleAddressMap.end(), 0};
    }
    
    if (auto it = _multiAddressMap.find(address); it != _multiAddressMap.end()) {
        return {address, AddressLocation::MultiUseMap, it, it->second};
    }
    
    if (auto it = _singleAddressMap.find(address); it != _singleAddressMap.end()) {
        return {address, AddressLocation::SingleUseMap, it, it->second};
    }
    
    if (auto it = _oldSingleAddressMap.find(address); it != _oldSingleAddressMap.end()) {
        return {address, AddressLocation::OldSingleUseMap, it, it->second};
    }
    
    leveldb::Slice keySlice(reinterpret_cast<const char *>(&address), sizeof(address));
    std::string value;
    leveldb::Status s = levelDb->Get(leveldb::ReadOptions(), keySlice, &value);
    if (s.ok()) {
        ldbcount++;
        uint32_t destNum = *reinterpret_cast<const uint32_t *>(value.data());
        return {address, AddressLocation::LevelDb, _singleAddressMap.end(), destNum};
    }
    fpcount++;
    // We must have had a false positive
    return {address, AddressLocation::NotFound, _singleAddressMap.end(), 0};
}

void AddressState::optionalSave() {
    if (singleAddressMap.size() > (SingleAddressMapMaxSize * 9) / 10) {
        clearAddressCache();
    }
}

void AddressState::clearAddressCache() {
    if (addressClearFuture.valid()) {
        addressClearFuture.get();
        oldSingleAddressMap.clear_no_resize();
    }
    singleAddressMap.swap(oldSingleAddressMap);
    addressClearFuture = std::async(std::launch::async, [&] {
        leveldb::WriteBatch batch;
        for (auto &entry : oldSingleAddressMap) {
            leveldb::Slice key(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first));
            leveldb::Slice value(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
            batch.Put(key, value);
        }
        levelDb->Write(leveldb::WriteOptions(), &batch);
    });
}
