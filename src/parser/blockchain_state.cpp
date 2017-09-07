//
//  blockchain_state.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/27/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "blockchain_state.hpp"
#include "preproccessed_block.hpp"

#include "parser_configuration.hpp"
#include "utilities.hpp"

#include <blocksci/hash.hpp>

#include <leveldb/write_batch.h>
#include <leveldb/cache.h>
#include <leveldb/env.h>
#include <leveldb/filter_policy.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>
#include <iostream>


const auto DeletedKeysMaxSize = 5000000;
const auto UTXOMapMaxSize = 5000000;
const auto SingleAddressMapMaxSize = 5000000;
const auto StartingAddressCount = 500'000'000;
const auto AddressFalsePositiveRate = .05;

UTXO::UTXO(const blocksci::Output &output_, const blocksci::Address &address_) : address(address_), output(output_) {}

BlockchainState::BlockchainState(const ParserConfiguration &config_) : config(config_)  {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB::Open(options, config.utxoDBPath().c_str(), &levelDb);

    blocksci::uint256 nullHash;
    nullHash.SetNull();
    RawOutputPointer deletedPointer = {nullHash, 0};
    utxoMap.set_deleted_key(deletedPointer);
    oldUTXOMap.set_deleted_key(deletedPointer);

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

BlockchainState::~BlockchainState() {
    boost::filesystem::ofstream bloomFile(config.addressBloomCacheFile(), std::ofstream::out | std::ofstream::binary);
    bloomFile << addressBloomFilter;
    
    auto file = fopen(config.addressCacheFile().c_str(), "wb");
    if (file != NULL) {
        multiAddressMap.serialize(address_map::NopointerSerializer(), file);
        fclose(file);
    }
    multiAddressMap.clear();
    
    saveScriptIndexes();
    
    clearDeletedKeys();
    clearUTXOCache();
    clearAddressCache();
    clearUTXOCache();
    clearAddressCache();
    
    if (utxoClearFuture.valid()) {
        utxoClearFuture.wait();
    }
    
    if (addressClearFuture.valid()) {
        addressClearFuture.wait();
    }
}

void BlockchainState::initializeScriptIndexes() {
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

void BlockchainState::saveScriptIndexes() {
    boost::filesystem::ofstream outputFile(config.scriptTypeCountFile());
    for (auto value : scriptIndexes) {
        outputFile << value << " ";
    }
}

uint32_t BlockchainState::getNewAddressIndex(blocksci::ScriptType::Enum type) {
    return ++scriptIndexes[static_cast<uint8_t>(type)];
}

UTXO BlockchainState::spendOutput(const RawOutputPointer &pointer) {
    auto it = utxoMap.find(pointer);
    if (it != utxoMap.end()) {
        UTXO utxo = it->second;
        utxoMap.erase(it);
        return utxo;
    }
    
    it = oldUTXOMap.find(pointer);
    if (it != oldUTXOMap.end()) {
        UTXO utxo = it->second;
        oldUTXOMap.erase(it);
        return utxo;
    }
    
    
    leveldb::Slice keySlice(reinterpret_cast<const char *>(&pointer), sizeof(pointer));
    std::string value;
    leveldb::Status s = levelDb->Get(leveldb::ReadOptions(), keySlice, &value);
    if (!s.ok()) {
        std::cout << "Couldn't find utxo for " << pointer.hash.GetHex() << std::endl;
        assert(false);
    }
    
    UTXO utxo = *reinterpret_cast<const UTXO *>(value.data());
    
    deletedKeys.push_back(pointer);
    return utxo;
}

BloomFilter<blocksci::RawAddress> BlockchainState::generateAddressBloomFilter(uint64_t maxAddresses, double falsePositiveRate) {
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

std::pair<uint32_t, bool> BlockchainState::resolveAddress(const AddressInfo &addressInfo) {
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
        default:
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

void BlockchainState::removeAddresses(const std::unordered_map<blocksci::ScriptType::Enum, uint32_t> &deletedIndex) {
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
    leveldb::Iterator* it = levelDb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        if (key.size() == sizeof(blocksci::RawAddress)) {
            uint32_t destNum = *reinterpret_cast<const uint32_t *>(it->value().data());
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

AddressInfo BlockchainState::findAddress(const blocksci::RawAddress &address) const {
    static uint64_t fpcount = 0;
    
    if (!addressBloomFilter.possiblyContains(address)) {
        // Address has definitely never been seen
        return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
    }
    
    auto multiAddressIt = multiAddressMap.find(address);
    if (multiAddressIt != multiAddressMap.end()) {
        return {address, AddressLocation::MultiUseMap, multiAddressIt, multiAddressIt->second};
    }
    
    auto singleAddressIt = singleAddressMap.find(address);
    if (singleAddressIt != singleAddressMap.end()) {
        return {address, AddressLocation::SingleUseMap, singleAddressIt, singleAddressIt->second};
    }
    
    auto singleAddressIt2 = oldSingleAddressMap.find(address);
    if (singleAddressIt2 != oldSingleAddressMap.end()) {
        return {address, AddressLocation::OldSingleUseMap, singleAddressIt2, singleAddressIt2->second};
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


void BlockchainState::addOutput(UTXO utxo, const RawOutputPointer &outputPointer) {
    utxoMap.insert(std::make_pair(outputPointer, utxo));
}

void BlockchainState::optionalSave() {
    if (deletedKeys.size() > DeletedKeysMaxSize) {
        clearDeletedKeys();
    }
    
    if (utxoMap.size() > (UTXOMapMaxSize * 9 / 2) / 10) {
        clearUTXOCache();
    }
    
    if (singleAddressMap.size() > (SingleAddressMapMaxSize * 9) / 10) {
        clearAddressCache();
    }
}

void BlockchainState::clearDeletedKeys() {
    leveldb::WriteBatch batch;
    for (auto &pointer : deletedKeys) {
        batch.Delete(leveldb::Slice(reinterpret_cast<const char *>(&pointer), sizeof(pointer)));
    }
    deletedKeys.clear();
    levelDb->Write(leveldb::WriteOptions(), &batch);
}

void BlockchainState::clearAddressCache() {
    if (addressClearFuture.valid()) {
        addressClearFuture.wait();
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

void BlockchainState::clearUTXOCache() {
    if (utxoClearFuture.valid()) {
        utxoClearFuture.wait();
        oldUTXOMap.clear();
        utxoMap.swap(oldUTXOMap);
    }
    
    auto utxosToSave = utxo_map(oldUTXOMap);
    utxoClearFuture = std::async(std::launch::async, [&, utxosToSave] {
        leveldb::WriteBatch batch;
        for (auto &entry : utxosToSave) {
            std::string value(reinterpret_cast<char const*>(&entry.second), sizeof(entry.second));
            batch.Put(leveldb::Slice(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first)), value);
        }
        levelDb->Write(leveldb::WriteOptions(), &batch);
    });
}
