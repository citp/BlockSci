//
//  address_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef address_state_hpp
#define address_state_hpp

#include "bloom_filter.hpp"
#include "parser_fwd.hpp"
#include "serializable_map.hpp"

#include <blocksci/util/state.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <rocksdb/db.h>

#include <future>
#include <unordered_map>

class CBloomFilter;

enum class AddressLocation {
    MultiUseMap,
    LevelDb,
    NotFound
};

struct RawScript {
    blocksci::uint160 hash;
    blocksci::ScriptType::Enum type;
    
    bool operator==(const RawScript& other) const {
        return type == other.type && hash == other.hash;
    }
};

namespace std {
    template <>
    struct hash<RawScript> {
    public:
        size_t operator()(const RawScript &b) const;
    };
}

template<blocksci::ScriptType::Enum type>
struct RawAddressInfo {
    blocksci::uint160 hash;
    AddressLocation location;
    uint32_t addressNum;
};

class AddressState {
public:
    class AddressMap : public SerializableMap<RawScript, uint32_t> {
    public:
        AddressMap();
    };
    
private:
    
    static constexpr auto StartingAddressCount = 500'000'000;
    static constexpr auto AddressFalsePositiveRate = .05;
    
    boost::filesystem::path path;
    
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    
    AddressMap multiAddressMap;
    BloomFilter addressBloomFilter;
    
    mutable long bloomNegativeCount = 0;
    mutable long multiCount = 0;
    mutable long dbCount = 0;
    mutable long bloomFPCount = 0;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    rocksdb::ColumnFamilyHandle *getColumn(blocksci::ScriptType::Enum type);
    
    void reloadBloomFilter();
    
public:
    AddressState(const boost::filesystem::path &path, const boost::filesystem::path &hashIndexPath);
    AddressState(const AddressState &) = delete;
    AddressState &operator=(const AddressState &) = delete;
    AddressState(AddressState &&) = delete;
    AddressState &operator=(AddressState &&) = delete;
    ~AddressState();
    
    template<blocksci::ScriptType::Enum type>
    RawAddressInfo<type> findAddress(const blocksci::uint160 &hash) {
        RawScript address{hash, type};
        if (!addressBloomFilter.possiblyContains(address)) {
            // Address has definitely never been seen
            bloomNegativeCount++;
            return {hash, AddressLocation::NotFound, 0};
        }
        
        {
            auto it = multiAddressMap.find(address);
            if (it != multiAddressMap.end()) {
                multiCount++;
                auto scriptNum = it->second;
                assert(scriptNum > 0);
                return {hash, AddressLocation::MultiUseMap, scriptNum};
            }
        }
        
        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&hash), sizeof(hash));
        std::string value;
        rocksdb::Status s = db->Get(rocksdb::ReadOptions(), getColumn(type), keySlice, &value);
        if (s.ok()) {
            dbCount++;
            uint32_t destNum;
            memcpy(&destNum, value.data(), sizeof(destNum));
            assert(destNum > 0);
            return {hash, AddressLocation::LevelDb, destNum};
        } else {
            bloomFPCount++;
            // We must have had a false positive
            return {hash, AddressLocation::NotFound, 0};
        }
    }
    
    // Bool is true if address is new
    template<blocksci::ScriptType::Enum type>
    std::pair<uint32_t, bool> resolveAddress(const RawAddressInfo<type> &addressInfo) {
        RawScript rawScript{addressInfo.hash, type};
        bool existingAddress = false;
        switch (addressInfo.location) {
            case AddressLocation::LevelDb:
                multiAddressMap.add(rawScript, addressInfo.addressNum);
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
            addressNum = getNewAddressIndex(type);
            addressBloomFilter.add(rawScript);
            rocksdb::Slice key(reinterpret_cast<const char *>(&addressInfo.hash), sizeof(addressInfo.hash));
            rocksdb::Slice value(reinterpret_cast<const char *>(&addressNum), sizeof(addressNum));
            db->Put(rocksdb::WriteOptions{}, getColumn(type), key, value);
            
            if (addressBloomFilter.isFull()) {
                addressBloomFilter.reset(addressBloomFilter.getMaxItems() * 2, addressBloomFilter.getFPRate());
                reloadBloomFilter();
            }
        }
        return std::make_pair(addressNum, !existingAddress);
    }
    
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void rollback(const blocksci::State &state);
};


#endif /* address_state_hpp */
