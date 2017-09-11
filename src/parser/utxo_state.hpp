//
//  utxo_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef utxo_state_hpp
#define utxo_state_hpp

#include "basic_types.hpp"
#include "utxo.hpp"

#include <sparsepp/spp.h>
#include <leveldb/db.h>

#include <future>
#include <stdio.h>

struct ParserConfiguration;

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

class UTXOState {
    using utxo_map = spp::sparse_hash_map<RawOutputPointer, UTXO, RawOutputPointerHasher>;
    
    const ParserConfiguration &config;
    leveldb::DB* levelDb;
    
    utxo_map utxoMap;
    utxo_map oldUTXOMap;
    
    std::vector<RawOutputPointer> deletedKeys;
    
    std::future<void> utxoClearFuture;
    
    void clearUTXOCache();
    void clearDeletedKeys();
    
public:
    UTXOState(const ParserConfiguration &config);
    ~UTXOState();
    
    void optionalSave();
    
    UTXO spendOutput(const RawOutputPointer &outputPointer);
    void addOutput(UTXO utxo, const RawOutputPointer &outputPointer);
};


#endif /* utxo_state_hpp */
