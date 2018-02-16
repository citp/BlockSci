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
    SingleUseMap,
    OldSingleUseMap,
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

struct AddressInfo;

class AddressState {
public:
    class AddressMap : public SerializableMap<RawScript, uint32_t> {
    public:
        AddressMap();
    };
    
private:
    
    static constexpr auto SingleAddressMapMaxSize = 20'000'000;
    static constexpr auto StartingAddressCount = 500'000'000;
    static constexpr auto AddressFalsePositiveRate = .05;
    
    boost::filesystem::path path;
    
    rocksdb::DB *db;
    std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    
    AddressMap multiAddressMap;
    AddressMap singleAddressMap;
    AddressMap oldSingleAddressMap;
    BloomFilter addressBloomFilter;
    
    mutable long bloomNegativeCount = 0;
    mutable long singleCount = 0;
    mutable long oldSingleCount = 0;
    mutable long multiCount = 0;
    mutable long levelDBCount = 0;
    mutable long bloomFPCount = 0;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    std::future<void> addressClearFuture;
    
    rocksdb::ColumnFamilyHandle *getColumn(blocksci::ScriptType::Enum type);
    
    void reloadBloomFilter();
    
    void clearAddressCache();
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    AddressState(const boost::filesystem::path &path, const boost::filesystem::path &hashIndexPath);
    AddressState(const AddressState &) = delete;
    AddressState &operator=(const AddressState &) = delete;
    AddressState(AddressState &&) = delete;
    AddressState &operator=(AddressState &&) = delete;
    ~AddressState();
    
    void optionalSave();

    AddressInfo findAddress(const RawScript &address);
    // Bool is true if address is new
    std::pair<uint32_t, bool> resolveAddress(const AddressInfo &addressInfo);
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void rollback(const blocksci::State &state);
};

struct AddressInfo {
    RawScript rawScript;
    AddressLocation location;
    AddressState::AddressMap::const_iterator it;
    uint32_t addressNum;
};


#endif /* address_state_hpp */
