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
    columnDescriptors.emplace_back("M", rocksdb::ColumnFamilyOptions{});
    columnDescriptors.emplace_back("T", rocksdb::ColumnFamilyOptions{});
    
    rocksdb::Status s = rocksdb::DB::Open(options, hashIndexPath.c_str(), columnDescriptors, &columnHandles, &db);
    
    assert(s.ok());
    
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
    std::cout << "multiCount: " << multiCount << "\n";
    std::cout << "dbCount: " << dbCount << "\n";
    std::cout << "bloomFPCount: " << bloomFPCount << "\n";
    
    for (auto handle : columnHandles) {
        delete handle;
    }
    delete db;
    
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
    case ScriptType::MULTISIG:
            return columnHandles[3];
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
    auto types = std::vector<ScriptType::Enum>{ScriptType::PUBKEY, ScriptType::SCRIPTHASH, ScriptType::MULTISIG};
    for (auto type : types) {
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), getColumn(type));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t scriptNum;
            memcpy(&scriptNum, it->value().data(), sizeof(scriptNum));
            uint160 addressHash;
            memcpy(&addressHash, it->key().data(), sizeof(addressHash));
            addressBloomFilter.add(RawScript{addressHash, type});
        }
        assert(it->status().ok());
        delete it;
    }
}

void AddressState::rollback(const blocksci::State &state) {
    for (auto multiAddressIt = multiAddressMap.begin(); multiAddressIt != multiAddressMap.end(); ++multiAddressIt) {
        auto count = state.scriptCounts[static_cast<size_t>(multiAddressIt->first.type)];
        if (multiAddressIt->second >= count) {
            multiAddressMap.erase(multiAddressIt);
        }
    }
    
    auto types = std::vector<ScriptType::Enum>{ScriptType::PUBKEY, ScriptType::SCRIPTHASH, ScriptType::MULTISIG};
    for (auto type : types) {
        auto column = getColumn(type);
        rocksdb::WriteBatch batch;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t destNum;
            memcpy(&destNum, it->value().data(), sizeof(destNum));
            auto count = state.scriptCounts[static_cast<size_t>(type)];
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
