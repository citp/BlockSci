//
//  bloom_filter.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/29/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "bloom_filter.hpp"
#include "MurmurHash3.hpp"

#include <blocksci/scripts/raw_script.hpp>
#include <blocksci/bitcoin_uint256.hpp>

#include <fstream>
#include <array>
#include <cmath>


constexpr double Log2 = 0.30102999566;
constexpr double Log2Squared = Log2 * Log2;

template<class Key>
BloomFilter<Key>::BloomFilter() : maxItems(0), fpRate(1), m_numHashes(0), length(0), addedCount(0), data(0)  {
}

template<class Key>
BloomFilter<Key>::BloomFilter(uint64_t maxItems_, double fpRate_) : maxItems(maxItems_), fpRate(fpRate_), m_numHashes(static_cast<uint8_t>(std::ceil(-std::log(fpRate) / Log2))), length(static_cast<uint64_t>(std::floor((-std::log(fpRate) * maxItems) / Log2Squared)) / sizeof(BlockType)), addedCount(0), data((length + BlockSize - 1) / BlockSize) {
    
}

inline std::array<uint64_t, 2> hash(const uint8_t *data, int len) {
    std::array<uint64_t, 2> hashValue;
    MurmurHash3_x64_128(data, len, 0, hashValue.data());
    
    return hashValue;
}

inline uint64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
    return (hashA + n * hashB) % filterSize;
}

template<class Key>
void BloomFilter<Key>::add(const Key &key) {
    int len = static_cast<int>(sizeof(Key));
    auto item = reinterpret_cast<const uint8_t *>(&key);
    auto hashValues = hash(item, len);
    
    for (uint8_t n = 0; n < m_numHashes; n++) {
        auto bitPos = nthHash(n, hashValues[0], hashValues[1], length);
        data[bitPos / BlockSize] |= BlockType{1} << (bitPos % BlockSize);
    }
    
    addedCount++;
}

template<class Key>
bool BloomFilter<Key>::possiblyContains(const Key &key) const {
    auto len = static_cast<int>(sizeof(Key));
    auto item = reinterpret_cast<const uint8_t *>(&key);
    auto hashValues = hash(item, len);
    
    for (uint8_t n = 0; n < m_numHashes; n++) {
        auto bitPos = nthHash(n, hashValues[0], hashValues[1], length);
        if ((data[bitPos / BlockSize] & (BlockType{1} << (bitPos % BlockSize))) == 0) {
            return false;
        }
    }
    
    return true;
}

template class BloomFilter<blocksci::RawScript>;
