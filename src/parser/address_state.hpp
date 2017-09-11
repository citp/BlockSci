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

#include <blocksci/address/raw_address_pointer.hpp>

#include <sparsepp/spp.h>
#include <leveldb/db.h>

#include <future>
#include <unordered_map>
#include <stdio.h>

struct ParserConfiguration;

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
    using address_map = spp::sparse_hash_map<blocksci::RawAddress, uint32_t, std::hash<blocksci::RawAddress>>;
private:
    const ParserConfiguration &config;
    leveldb::DB* levelDb;
    
    address_map multiAddressMap;
    address_map singleAddressMap;
    address_map oldSingleAddressMap;
    BloomFilter<blocksci::RawAddress> addressBloomFilter;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    std::future<void> addressClearFuture;
    
    BloomFilter<blocksci::RawAddress> generateAddressBloomFilter(uint64_t maxAddresses, double falsePositiveRate);
    
    void initializeScriptIndexes();
    void saveScriptIndexes();
    
    void clearAddressCache();
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    AddressState(const ParserConfiguration &config);
    ~AddressState();
    
    void optionalSave();

    AddressInfo findAddress(const blocksci::RawAddress &address) const;
    // Bool is true if address is new
    std::pair<uint32_t, bool> resolveAddress(const AddressInfo &addressInfo);
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void removeAddresses(const std::unordered_map<blocksci::ScriptType::Enum, uint32_t> &deletedIndex);
};

struct AddressInfo {
    blocksci::RawAddress rawAddress;
    AddressLocation location;
    AddressState::address_map::const_iterator it;
    uint32_t addressNum;
};


#endif /* address_state_hpp */
