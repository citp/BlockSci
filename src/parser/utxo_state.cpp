//
//  utxo_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "utxo_state.hpp"
#include "parser_configuration.hpp"

#include <leveldb/write_batch.h>
#include <iostream>

UTXOState::UTXOState(const ParserConfigurationBase &config_) : config(config_)  {
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    RawOutputPointer deletedPointer = {nullHash, 0};
    utxoMap.set_deleted_key(deletedPointer);
    
    auto file = fopen(config.utxoCacheFile().c_str(), "rb");
    if (file != NULL) {
        utxoMap.unserialize(utxo_map::NopointerSerializer(), file);
        fclose(file);
    }
}

UTXOState::~UTXOState() {
    auto file = fopen(config.utxoCacheFile().c_str(), "wb");
    if (file != NULL) {
        utxoMap.serialize(utxo_map::NopointerSerializer(), file);
        fclose(file);
    }
}

UTXO UTXOState::spendOutput(const RawOutputPointer &pointer) {
    auto it = utxoMap.find(pointer);
    if (it == utxoMap.end()) {
        throw std::runtime_error("Tried to spend nonexistant utxo");
    }
    
    UTXO utxo = it->second;
    utxoMap.erase(it);
    return utxo;
}


void UTXOState::addOutput(UTXO utxo, const RawOutputPointer &outputPointer) {
    utxoMap.insert(std::make_pair(outputPointer, utxo));
}
