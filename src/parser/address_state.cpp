//
//  address_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "address_state.hpp"
#include "parser_configuration.hpp"

#include <leveldb/write_batch.h>

#include <boost/filesystem/fstream.hpp>

const auto SingleAddressMapMaxSize = 10'000'000;
const auto StartingAddressCount = 500'000'000;
const auto AddressFalsePositiveRate = .05;

AddressState::AddressState(const ParserConfiguration &config_) : config(config_)  {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB::Open(options, config.addressDBPath().c_str(), &levelDb);
    
    blocksci::uint160 deletedAddress;
    deletedAddress.SetHex("FFFFFFFFFFFFFFFFFFFF");
    blocksci::RawAddress deletedKey{deletedAddress, blocksci::ScriptType::Enum::NULL_DATA};
    
    singleAddressMap.set_deleted_key(deletedKey);
    oldSingleAddressMap.set_deleted_key(deletedKey);
    multiAddressMap.set_deleted_key(deletedKey);
    
    auto file = fopen(config.addressCacheFile().c_str(), "rb");
    if (file != NULL) {
        multiAddressMap.unserialize(address_map::NopointerSerializer(), file);
    }
    
    boost::filesystem::ifstream bloomFile(config.addressBloomCacheFile(), std::ofstream::in | std::ofstream::binary);
    if (bloomFile.is_open()) {
        bloomFile >> addressBloomFilter;
    } else {
        addressBloomFilter = generateAddressBloomFilter(StartingAddressCount, AddressFalsePositiveRate);
    }
    
    initializeScriptIndexes();
}

AddressState::~AddressState() {
    boost::filesystem::ofstream bloomFile(config.addressBloomCacheFile(), std::ofstream::out | std::ofstream::binary);
    bloomFile << addressBloomFilter;
    
    auto file = fopen(config.addressCacheFile().c_str(), "wb");
    if (file != NULL) {
        multiAddressMap.serialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    multiAddressMap.clear();
    
    saveScriptIndexes();
    
    clearAddressCache();
    clearAddressCache();
    
    if (addressClearFuture.valid()) {
        addressClearFuture.get();
    }
}

void AddressState::initializeScriptIndexes() {
    boost::filesystem::ifstream inputFile(config.scriptTypeCountFile());
    
    if (inputFile) {
        uint32_t value;
        while ( inputFile >> value ) {
            scriptIndexes.push_back(value);
        }
    } else {
        for (size_t i = 0; i < blocksci::ScriptType::all.size(); i++) {
            scriptIndexes.push_back(0);
        }
    }
}

void AddressState::saveScriptIndexes() {
    boost::filesystem::ofstream outputFile(config.scriptTypeCountFile());
    for (auto value : scriptIndexes) {
        outputFile << value << " ";
    }
}

uint32_t AddressState::getNewAddressIndex(blocksci::ScriptType::Enum type) {
    return ++scriptIndexes[static_cast<uint8_t>(type)];
}

BloomFilter<blocksci::RawAddress> AddressState::generateAddressBloomFilter(uint64_t maxAddresses, double falsePositiveRate) {
    BloomFilter<blocksci::RawAddress> bloom{maxAddresses, falsePositiveRate};
    for (auto &pair : singleAddressMap) {
        bloom.add(pair.first);
    }
    
    for (auto &pair : oldSingleAddressMap) {
        bloom.add(pair.first);
    }
    
    for (auto &pair : multiAddressMap) {
        bloom.add(pair.first);
    }
    
    leveldb::Iterator* it = levelDb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        if (key.size() == sizeof(blocksci::RawAddress)) {
            auto address = *reinterpret_cast<const blocksci::RawAddress *>(key.data());
            bloom.add(address);
        }
    }
    
    return bloom;
}

std::pair<uint32_t, bool> AddressState::resolveAddress(const AddressInfo &addressInfo) {
    auto rawAddress = addressInfo.rawAddress;
    switch (addressInfo.location) {
        case AddressLocation::SingleUseMap:
            singleAddressMap.erase(addressInfo.it);
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            break;
        case AddressLocation::OldSingleUseMap:
            oldSingleAddressMap.erase(addressInfo.it);
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            break;
        case AddressLocation::LevelDb:
            multiAddressMap.insert(std::make_pair(rawAddress, addressInfo.addressNum));
            break;
        case AddressLocation::MultiUseMap:
            break;
        case AddressLocation::NotFound:
            break;
    }
    
    uint32_t addressNum = addressInfo.addressNum;
    bool foundAddress = addressNum != 0;
    if (!foundAddress) {
        addressNum = getNewAddressIndex(rawAddress.type);
        addressBloomFilter.add(rawAddress);
        singleAddressMap.insert(std::make_pair(rawAddress, addressNum));
        
        if (addressBloomFilter.isFull()) {
            addressBloomFilter = generateAddressBloomFilter(addressBloomFilter.getMaxItems() * 2, addressBloomFilter.getFPRate());
        }
    }
    return std::make_pair(addressNum, !foundAddress);
}

void AddressState::removeAddresses(const std::unordered_map<blocksci::ScriptType::Enum, uint32_t> &deletedIndex) {
    auto multiAddressIt = multiAddressMap.begin();
    while (multiAddressIt != multiAddressMap.end()) {
        auto it = deletedIndex.find(multiAddressIt->first.type);
        if (it != deletedIndex.end() && multiAddressIt->second >= it->second) {
            multiAddressIt = multiAddressMap.erase(multiAddressIt);
        } else {
            ++multiAddressIt;
        }
    }
    
    auto singleAddressIt = singleAddressMap.begin();
    while (singleAddressIt != singleAddressMap.end()) {
        auto it = deletedIndex.find(singleAddressIt->first.type);
        if (it != deletedIndex.end() && singleAddressIt->second >= it->second) {
            singleAddressIt = singleAddressMap.erase(singleAddressIt);
        } else {
            ++singleAddressIt;
        }
    }
    
    auto singleAddressIt2 = oldSingleAddressMap.begin();
    while (singleAddressIt2 != oldSingleAddressMap.end()) {
        auto it = deletedIndex.find(singleAddressIt2->first.type);
        if (it != deletedIndex.end() && singleAddressIt2->second >= it->second) {
            singleAddressIt2 = oldSingleAddressMap.erase(singleAddressIt2);
        } else {
            ++singleAddressIt2;
        }
    }
    
    leveldb::WriteBatch batch;
    leveldb::Iterator* levelDbIt = levelDb->NewIterator(leveldb::ReadOptions());
    for (levelDbIt->SeekToFirst(); levelDbIt->Valid(); levelDbIt->Next()) {
        auto key = levelDbIt->key();
        if (key.size() == sizeof(blocksci::RawAddress)) {
            uint32_t destNum = *reinterpret_cast<const uint32_t *>(levelDbIt->value().data());
            auto address = *reinterpret_cast<const blocksci::RawAddress *>(key.data());
            auto it = deletedIndex.find(address.type);
            if (it != deletedIndex.end() && singleAddressIt2->second >= destNum) {
                batch.Delete(key);
            }
        }
    }
    levelDb->Write(leveldb::WriteOptions(), &batch);
    
    addressBloomFilter = generateAddressBloomFilter(addressBloomFilter.getMaxItems(), addressBloomFilter.getFPRate());
}

AddressInfo AddressState::findAddress(const blocksci::RawAddress &address) const {
    static uint64_t fpcount = 0;
    
    if (!addressBloomFilter.possiblyContains(address)) {
        // Address has definitely never been seen
        return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
    }
    
    if (auto it = multiAddressMap.find(address); it != multiAddressMap.end()) {
        return {address, AddressLocation::MultiUseMap, it, it->second};
    }
    
    if (auto it = singleAddressMap.find(address); it != singleAddressMap.end()) {
        return {address, AddressLocation::SingleUseMap, it, it->second};
    }
    
    if (auto it = oldSingleAddressMap.find(address); it != oldSingleAddressMap.end()) {
        return {address, AddressLocation::OldSingleUseMap, it, it->second};
    }
    
    leveldb::Slice keySlice(reinterpret_cast<const char *>(&address), sizeof(address));
    std::string value;
    leveldb::Status s = levelDb->Get(leveldb::ReadOptions(), keySlice, &value);
    if (s.ok()) {
        uint32_t destNum = *reinterpret_cast<const uint32_t *>(value.data());
        return {address, AddressLocation::LevelDb, singleAddressMap.end(), destNum};
    }
    fpcount++;
    // We must have had a false positive
    return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
}

void AddressState::optionalSave() {
    if (singleAddressMap.size() > (SingleAddressMapMaxSize * 9) / 10) {
        clearAddressCache();
    }
}

void AddressState::clearAddressCache() {
    if (addressClearFuture.valid()) {
        addressClearFuture.get();
        oldSingleAddressMap.clear();
        singleAddressMap.swap(oldSingleAddressMap);
    }
    auto addressesToSave = oldSingleAddressMap;
    addressClearFuture = std::async(std::launch::async, [&, addressesToSave] {
        leveldb::WriteBatch batch;
        for (auto &entry : addressesToSave) {
            std::string value(reinterpret_cast<char const*>(&entry.second), sizeof(entry.second));
            batch.Put(leveldb::Slice(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first)), value);
        }
        levelDb->Write(leveldb::WriteOptions(), &batch);
    });
}
