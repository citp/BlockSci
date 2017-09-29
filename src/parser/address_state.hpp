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

#include <blocksci/scripts/raw_script.hpp>

#include <sparsepp/spp.h>
#include <leveldb/db.h>

#include <future>
#include <unordered_map>
#include <stdio.h>

class CBloomFilter;

enum class AddressLocation {
    SingleUseMap,
    OldSingleUseMap,
    MultiUseMap,
    LevelDb,
    NotFound
};



struct AddressInfo;

class AddressState {
public:
    using address_map = spp::sparse_hash_map<blocksci::RawScript, uint32_t, std::hash<blocksci::RawScript>>;
private:
    const ParserConfigurationBase &config;
    leveldb::DB* levelDb;
    
    address_map multiAddressMap;
    address_map singleAddressMap;
    address_map oldSingleAddressMap;
    BloomFilter<blocksci::RawScript> addressBloomFilter;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    std::future<void> addressClearFuture;
    
    BloomFilter<blocksci::RawScript> generateAddressBloomFilter(uint64_t maxAddresses, double falsePositiveRate);
    
    void initializeScriptIndexes();
    void saveScriptIndexes();
    
    void clearAddressCache();
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    AddressState(const ParserConfigurationBase &config);
    ~AddressState();
    
    void optionalSave();

    AddressInfo findAddress(const blocksci::RawScript &address) const;
    // Bool is true if address is new
    std::pair<uint32_t, bool> resolveAddress(const AddressInfo &addressInfo);
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void removeAddresses(const std::unordered_map<blocksci::ScriptType::Enum, uint32_t> &deletedIndex);
};

struct AddressInfo {
    blocksci::RawScript rawAddress;
    AddressLocation location;
    AddressState::address_map::const_iterator it;
    uint32_t addressNum;
};


#endif /* address_state_hpp */
