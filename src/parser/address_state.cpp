//
//  address_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "address_state.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util/hash.hpp>

#include <hyperleveldb/db.h>
#include <hyperleveldb/write_batch.h>

#include <boost/filesystem/fstream.hpp>

#include <string_view>
#include <iostream>

namespace {
    using namespace std::string_view_literals;
    
    static constexpr auto singleAddressFileName = "single.dat"sv;
    static constexpr auto multiAddressFileName = "multi.dat"sv;
    static constexpr auto dbFileName = "db"sv;
    static constexpr auto bloomFileName = "bloom"sv;
    static constexpr auto scriptCountsFileName = "scriptCounts.txt"sv;
}

namespace std {
    size_t hash<RawScript>::operator()(const RawScript &b) const {
        std::size_t seed = 8957643;
        
        hash_combine(seed, b.hash);
        hash_combine(seed, b.type);
        return seed;
    };
}

AddressState::AddressMap::AddressMap() : SerializableMap<RawScript, uint32_t>({blocksci::uint160S("FFFFFFFFFFFFFFFFFFFF"), blocksci::ScriptType::Enum::NULL_DATA}, {blocksci::uint160S("FFFFFFFFFFFFFFFFFFFF"), blocksci::ScriptType::Enum::SCRIPTHASH}) {}


AddressState::AddressState(const boost::filesystem::path &path_) : path(path_), addressBloomFilter(path/std::string(bloomFileName), StartingAddressCount, AddressFalsePositiveRate)  {
    leveldb::Options options;
    options.create_if_missing = true;
    options.write_buffer_size = 128 * 1024* 1024;
    leveldb::DB::Open(options, (path/std::string(dbFileName)).c_str(), &levelDb);
    
    singleAddressMap.resize(SingleAddressMapMaxSize);
    oldSingleAddressMap.resize(SingleAddressMapMaxSize);
    multiAddressMap.resize(SingleAddressMapMaxSize);
    
    singleAddressMap.unserialize((path/std::string(singleAddressFileName)).native());
    multiAddressMap.unserialize((path/std::string(multiAddressFileName)).native());
    
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
    
    std::cout << "\nbloomNegativeCount: " << bloomNegativeCount << "\n";
    std::cout << "singleCount: " << singleCount << "\n";
    std::cout << "oldSingleCount: " << oldSingleCount << "\n";
    std::cout << "multiCount: " << multiCount << "\n";
    std::cout << "levelDBCount: " << levelDBCount << "\n";
    std::cout << "bloomFPCount: " << bloomFPCount << "\n";
    
    if (addressClearFuture.valid()) {
        addressClearFuture.get();
    }
    
    singleAddressMap.serialize((path/std::string(singleAddressFileName)).native());
    multiAddressMap.serialize((path/std::string(multiAddressFileName)).native());
    
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
        if (key.size() == sizeof(RawScript)) {
            auto address = *reinterpret_cast<const RawScript *>(key.data());
            addressBloomFilter.add(address);
        }
    }
}

AddressInfo AddressState::findAddress(const RawScript &address) const {
    
    if (!addressBloomFilter.possiblyContains(address)) {
        // Address has definitely never been seen
        bloomNegativeCount++;
        return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
    }
    
    if (auto it = multiAddressMap.find(address); it != multiAddressMap.end()) {
        multiCount++;
        auto scriptNum = it->second;
        assert(scriptNum > 0);
        return {address, AddressLocation::MultiUseMap, it, scriptNum};
    }
    
    if (auto it = singleAddressMap.find(address); it != singleAddressMap.end()) {
        singleCount++;
        auto scriptNum = it->second;
        assert(scriptNum > 0);
        return {address, AddressLocation::SingleUseMap, it, scriptNum};
    }
    
    if (auto it = oldSingleAddressMap.find(address); it != oldSingleAddressMap.end()) {
        oldSingleCount++;
        auto scriptNum = it->second;
        assert(scriptNum > 0);
        return {address, AddressLocation::OldSingleUseMap, it, scriptNum};
    }
    
    leveldb::Slice keySlice(reinterpret_cast<const char *>(&address), sizeof(address));
    std::string value;
    leveldb::Status s = levelDb->Get(leveldb::ReadOptions(), keySlice, &value);
    if (s.ok()) {
        levelDBCount++;
        uint32_t destNum = *reinterpret_cast<const uint32_t *>(value.data());
        assert(destNum > 0);
        return {address, AddressLocation::LevelDb, singleAddressMap.end(), destNum};
    }
    bloomFPCount++;
    // We must have had a false positive
    return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
}

std::pair<uint32_t, bool> AddressState::resolveAddress(const AddressInfo &addressInfo) {
    auto rawScript = addressInfo.rawScript;
    bool existingAddress = false;
    switch (addressInfo.location) {
        case AddressLocation::SingleUseMap:
            singleAddressMap.erase(addressInfo.it);
            multiAddressMap.add(rawScript, addressInfo.addressNum);
            assert(addressInfo.addressNum > 0);
            existingAddress = true;
            break;
        case AddressLocation::OldSingleUseMap:
            oldSingleAddressMap.erase(addressInfo.it);
            multiAddressMap.add(rawScript, addressInfo.addressNum);
            assert(addressInfo.addressNum > 0);
            existingAddress = true;
            break;
        case AddressLocation::LevelDb:
            multiAddressMap.add(rawScript, addressInfo.addressNum);
            assert(addressInfo.addressNum > 0);
            existingAddress = true;
            break;
        case AddressLocation::MultiUseMap:
            assert(addressInfo.addressNum > 0);
            existingAddress = true;
            break;
        case AddressLocation::NotFound:
            existingAddress = false;
            break;
    }
    
    uint32_t addressNum = addressInfo.addressNum;
    if (!existingAddress) {
        addressNum = getNewAddressIndex(rawScript.type);
        addressBloomFilter.add(rawScript);
        singleAddressMap.add(rawScript, addressNum);
        
        if (addressBloomFilter.isFull()) {
            addressBloomFilter.reset(addressBloomFilter.getMaxItems() * 2, addressBloomFilter.getFPRate());
            reloadBloomFilter();
        }
    }
    assert(addressNum > 0);
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
        if (key.size() == sizeof(RawScript)) {
            uint32_t destNum = *reinterpret_cast<const uint32_t *>(levelDbIt->value().data());
            auto address = *reinterpret_cast<const RawScript *>(key.data());
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
