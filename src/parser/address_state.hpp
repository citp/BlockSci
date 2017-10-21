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

#include <blocksci/state.hpp>
#include <blocksci/scripts/raw_script.hpp>

#include <google/dense_hash_map>

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

namespace leveldb {
    class DB;
}

struct AddressInfo;

class AddressState {
public:
    using address_map = google::dense_hash_map<blocksci::RawScript, uint32_t, std::hash<blocksci::RawScript>>;
private:
    
    static constexpr auto SingleAddressMapMaxSize = 10'000'000;
    static constexpr auto StartingAddressCount = 5'000'000;
    static constexpr auto AddressFalsePositiveRate = .05;
    
    boost::filesystem::path path;
    
    leveldb::DB* levelDb;
    
    address_map multiAddressMap;
    address_map singleAddressMap;
    address_map oldSingleAddressMap;
    BloomFilter addressBloomFilter;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    std::future<void> addressClearFuture;
    
    void reloadBloomFilter();
    
    void clearAddressCache();
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    AddressState(const boost::filesystem::path &path);
    AddressState(const AddressState &) = delete;
    AddressState &operator=(const AddressState &) = delete;
    AddressState(AddressState &&) = delete;
    AddressState &operator=(AddressState &&) = delete;
    ~AddressState();
    
    void optionalSave();

    AddressInfo findAddress(const blocksci::RawScript &address) const;
    // Bool is true if address is new
    std::pair<uint32_t, bool> resolveAddress(const AddressInfo &addressInfo);
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void rollback(const blocksci::State &state);
};

struct AddressInfo {
    blocksci::RawScript rawAddress;
    AddressLocation location;
    AddressState::address_map::iterator it;
    uint32_t addressNum;
};


#endif /* address_state_hpp */
