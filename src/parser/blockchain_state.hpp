//
//  blockchain_state.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/27/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blockchain_state_hpp
#define blockchain_state_hpp

#include "bloom_filter.hpp"
#include "basic_types.hpp"

#include <blocksci/bitcoin_uint256.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/raw_address_pointer.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/output.hpp>

#include <sparsepp/spp.h>

#include <sqlite3.h>
#include <leveldb/db.h>

#include <unordered_map>
#include <future>
#include <array>
#include <stdio.h>

struct ParserConfiguration;

struct UTXO {
    blocksci::Address address;
    blocksci::Output output;
    
    UTXO() = default;
    UTXO(const blocksci::Output &output, const blocksci::Address &address);
};

class CBloomFilter;


struct RawOutputPointerHasher {
    size_t operator()(const RawOutputPointer& b) const {
        std::size_t seed = 123945432;
        std::hash<blocksci::uint256> hasher1;
        std::hash<uint16_t> hasher2;
        seed ^= hasher1(b.hash) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hasher2(b.outputNum) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

enum class AddressLocation {
    SingleUseMap,
    OldSingleUseMap,
    MultiUseMap,
    LevelDb,
    NotFound
};

using address_map = spp::sparse_hash_map<blocksci::RawAddress, uint32_t, std::hash<blocksci::RawAddress>>;

struct AddressInfo {
    blocksci::RawAddress rawAddress;
    AddressLocation location;
    address_map::const_iterator it;
    uint32_t addressNum;
};

class BlockchainState {
    using utxo_map = spp::sparse_hash_map<RawOutputPointer, UTXO, RawOutputPointerHasher>;
    
    const ParserConfiguration &config;
    leveldb::DB* levelDb;
    
    utxo_map utxoMap;
    utxo_map oldUTXOMap;
    
    address_map multiAddressMap;
    address_map singleAddressMap;
    address_map oldSingleAddressMap;
    BloomFilter<blocksci::RawAddress> addressBloomFilter;

    std::vector<RawOutputPointer> deletedKeys;
    
    std::vector<uint32_t> scriptIndexes;
    
    std::future<void> addressClearFuture;
    std::future<void> utxoClearFuture;
    
    BloomFilter<blocksci::RawAddress> generateAddressBloomFilter(uint64_t maxAddresses, double falsePositiveRate);
    
    void initializeScriptIndexes();
    void saveScriptIndexes();
    
    void clearAddressCache();
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    BlockchainState(const ParserConfiguration &config);
    ~BlockchainState();
    
    void optionalSave();
    
    UTXO spendOutput(const RawOutputPointer &outputPointer);
    void addOutput(UTXO utxo, const RawOutputPointer &outputPointer);
    
    AddressInfo findAddress(const blocksci::RawAddress &address) const;
    // Bool is true if address is new
    std::pair<uint32_t, bool> resolveAddress(const AddressInfo &addressInfo);
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void removeAddresses(const std::unordered_map<blocksci::ScriptType::Enum, uint32_t> &deletedIndex);
};

#endif /* blockchain_state_hpp */
