//
//  address_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "address_state.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/util/hash.hpp>

#include <boost/filesystem/fstream.hpp>

#include <iostream>

using namespace blocksci;

namespace {
    static constexpr auto singleAddressFileName = "single.dat";
    static constexpr auto multiAddressFileName = "multi.dat";
    static constexpr auto dbFileName = "db";
    static constexpr auto bloomFileName = "bloom";
    static constexpr auto scriptCountsFileName = "scriptCounts.txt";
}

namespace std {
    size_t hash<RawScript>::operator()(const RawScript &b) const {
        std::size_t seed = 8957643;
        
        hash_combine(seed, b.hash);
        hash_combine(seed, b.type);
        return seed;
    }
}

AddressState::AddressMap::AddressMap() : SerializableMap<RawScript, uint32_t>({blocksci::uint160S("FFFFFFFFFFFFFFFFFFFF"), blocksci::ScriptType::Enum::NULL_DATA}, {blocksci::uint160S("FFFFFFFFFFFFFFFFFFFF"), blocksci::ScriptType::Enum::SCRIPTHASH}) {}

AddressState::AddressState(const boost::filesystem::path &path_, const boost::filesystem::path &hashIndexPath) : path(path_), addressBloomFilter(path/std::string(bloomFileName), StartingAddressCount, AddressFalsePositiveRate)  {
    rocksdb::Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    
    
    std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
    columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
    columnDescriptors.emplace_back("P", rocksdb::ColumnFamilyOptions{});
    columnDescriptors.emplace_back("S", rocksdb::ColumnFamilyOptions{});
    columnDescriptors.emplace_back("T", rocksdb::ColumnFamilyOptions{});
    
    rocksdb::Status s = rocksdb::DB::Open(options, hashIndexPath.c_str(), columnDescriptors, &columnHandles, &db);
    
    assert(s.ok());
    
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
    
    rocksdb::WriteBatch batch;
    for (auto &entry : singleAddressMap) {
        rocksdb::Slice key(reinterpret_cast<const char *>(&entry.first.hash), sizeof(entry.first.hash));
        rocksdb::Slice value(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
        batch.Put(getColumn(entry.first.type), key, value);
    }
    db->Write(rocksdb::WriteOptions(), &batch);
    
    for (auto handle : columnHandles) {
        delete handle;
    }
    delete db;
    
    singleAddressMap.serialize((path/std::string(singleAddressFileName)).native());
    multiAddressMap.serialize((path/std::string(multiAddressFileName)).native());
    
    boost::filesystem::ofstream outputFile(path/std::string(scriptCountsFileName));
    for (auto value : scriptIndexes) {
        outputFile << value << " ";
    }
}

rocksdb::ColumnFamilyHandle *AddressState::getColumn(ScriptType::Enum type) {
    switch (type) {
    case ScriptType::PUBKEY:
        return columnHandles[1];
    case ScriptType::SCRIPTHASH:
        return columnHandles[2];
    default:
        assert("Tried to get column for unindexed script type");
    }
    return nullptr;
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
    
    {
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), getColumn(ScriptType::Enum::PUBKEY));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t scriptNum;
            memcpy(&scriptNum, it->value().data(), sizeof(scriptNum));
            uint160 addressHash;
            memcpy(&addressHash, it->key().data(), sizeof(addressHash));
            addressBloomFilter.add(RawScript{addressHash, ScriptType::Enum::PUBKEY});
        }
        assert(it->status().ok());
        delete it;
    }
    
    {
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), getColumn(ScriptType::Enum::SCRIPTHASH));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t scriptNum;
            memcpy(&scriptNum, it->value().data(), sizeof(scriptNum));
            uint160 addressHash;
            memcpy(&addressHash, it->key().data(), sizeof(addressHash));
            addressBloomFilter.add(RawScript{addressHash, ScriptType::Enum::SCRIPTHASH});
        }
        assert(it->status().ok());
        delete it;
    }
}

AddressInfo AddressState::findAddress(const RawScript &address) {
    
    if (!addressBloomFilter.possiblyContains(address)) {
        // Address has definitely never been seen
        bloomNegativeCount++;
        return {address, AddressLocation::NotFound, singleAddressMap.end(), 0};
    }
    
    {
        auto it = multiAddressMap.find(address);
        if (it != multiAddressMap.end()) {
            multiCount++;
            auto scriptNum = it->second;
            assert(scriptNum > 0);
            return {address, AddressLocation::MultiUseMap, it, scriptNum};
        }
    }
    
    {
        auto it = singleAddressMap.find(address);
        if (it != singleAddressMap.end()) {
            singleCount++;
            auto scriptNum = it->second;
            assert(scriptNum > 0);
            return {address, AddressLocation::SingleUseMap, it, scriptNum};
        }
    }
    
    {
        auto it = oldSingleAddressMap.find(address);
        if (it != oldSingleAddressMap.end()) {
            oldSingleCount++;
            auto scriptNum = it->second;
            assert(scriptNum > 0);
            return {address, AddressLocation::OldSingleUseMap, it, scriptNum};
        }
    }
    
    
    rocksdb::Slice keySlice(reinterpret_cast<const char *>(&address.hash), sizeof(address.hash));
    std::string value;
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), getColumn(address.type), keySlice, &value);
    if (s.ok()) {
        levelDBCount++;
        uint32_t destNum;
        memcpy(&destNum, value.data(), sizeof(destNum));
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
            multiAddressMap.add(rawScript, addressInfo.addressNum);
            assert(addressInfo.addressNum > 0);
            existingAddress = true;
            break;
        case AddressLocation::OldSingleUseMap:
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
    
    {
        auto column = getColumn(ScriptType::SCRIPTHASH);
        rocksdb::WriteBatch batch;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t destNum;
            memcpy(&destNum, it->value().data(), sizeof(destNum));
            uint160 addressHash;
            memcpy(&addressHash, it->key().data(), sizeof(addressHash));
            auto count = state.scriptCounts[static_cast<size_t>(ScriptType::SCRIPTHASH)];
            if (destNum >= count) {
                batch.Delete(column, it->key());
            }
        }
        assert(it->status().ok());
        delete it;
        db->Write(rocksdb::WriteOptions(), &batch);
    }
    
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
        rocksdb::WriteBatch batch;
        for (auto &entry : oldSingleAddressMap) {
            rocksdb::Slice key(reinterpret_cast<const char *>(&entry.first.hash), sizeof(entry.first.hash));
            rocksdb::Slice value(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
            batch.Put(getColumn(entry.first.type), key, value);
        }
        db->Write(rocksdb::WriteOptions(), &batch);
    });
}
