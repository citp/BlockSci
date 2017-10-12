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

#include <leveldb/write_batch.h>
#include <iostream>

class FileSerializer
{
public:
    // serialize basic types to FILE
    // -----------------------------
    template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    bool operator()(std::ofstream *fp, const T& value)
    {
        fp->write(reinterpret_cast<const char *>(&value), sizeof(value));
        return fp->good();
    }
    
    template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    bool operator()(std::ifstream *fp, T* value)
    {
        fp->read(reinterpret_cast<char *>(value), sizeof(*value));
        return fp->good();
    }
    
    // serialize std::pair<const A, B> to FILE - needed for maps
    // ---------------------------------------------------------
    template <class A, class B>
    bool operator()(std::ofstream *fp, const std::pair<const A, B>& value)
    {
        return this->operator()(fp, value.first) && this->operator()(fp, value.second);
    }
    
    template <class A, class B>
    bool operator()(std::ifstream *fp, std::pair<const A, B> *value)
    {
        const A *constFirst = &value->first;
        A *first = const_cast<A *>(constFirst);
        return this->operator()(fp, first) && this->operator()(fp, &value->second);
    }
};

UTXOState::UTXOState(const ParserConfigurationBase &config_) : config(config_)  {
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    RawOutputPointer deletedPointer = {nullHash, 0};
    utxoMap.set_deleted_key(deletedPointer);
    RawOutputPointer emptyPointer = {nullHash, 1};
    utxoMap.set_empty_key(emptyPointer);
    
    boost::filesystem::ifstream file{config.utxoCacheFile(), std::fstream::in | std::fstream::binary};
    if (file.is_open()) {
        auto success = utxoMap.unserialize(FileSerializer(), &file);
        assert(success);
    }
}

UTXOState::~UTXOState() {
    boost::filesystem::ofstream file{config.utxoCacheFile(), std::fstream::out | std::fstream::binary};
    auto success = utxoMap.serialize(FileSerializer(), &file);
    assert(success);
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
