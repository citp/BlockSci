//
//  utxo_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "utxo_state.hpp"
#include "parser_configuration.hpp"

#include <boost/filesystem/fstream.hpp>

#include <iostream>

UTXOState::UTXOState()  {
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    RawOutputPointer deletedPointer = {nullHash, 0};
    utxoMap.set_deleted_key(deletedPointer);
    RawOutputPointer emptyPointer = {nullHash, 1};
    utxoMap.set_empty_key(emptyPointer);
}

bool UTXOState::unserialize(const boost::filesystem::path &path) {
    boost::filesystem::ifstream file{path, std::fstream::in | std::fstream::binary};
    if (file.is_open()) {
        return utxoMap.unserialize(utxo_map::NopointerSerializer(), &file);
    }
    return false;
}

bool UTXOState::serialize(const boost::filesystem::path &path) {
    boost::filesystem::ofstream file{path, std::fstream::out | std::fstream::binary};
    return utxoMap.serialize(utxo_map::NopointerSerializer(), &file);
}

UTXO UTXOState::spendOutput(const RawOutputPointer &pointer) {
    auto it = utxoMap.find(pointer);
    if (it == utxoMap.end()) {
        throw UTXOMissingException{pointer};
    }
    
    UTXO utxo = it->second;
    utxoMap.erase(it);
    return utxo;
}


void UTXOState::addOutput(UTXO utxo, const RawOutputPointer &outputPointer) {
    utxoMap.insert(std::make_pair(outputPointer, utxo));
}
