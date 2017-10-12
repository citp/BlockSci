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

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <fstream>
#include <array>
#include <cmath>


constexpr double Log2 = 0.30102999566;
constexpr double Log2Squared = Log2 * Log2;

BloomStore::BloomStore(const boost::filesystem::path &path, uint64_t length_) : backingFile(path), length(length_) {
    if (backingFile.size() == 0) {
        backingFile.truncate(blockCount());
    }
    
    if (backingFile.size() != blockCount()) {
        throw std::runtime_error("Trying to open bloom filter of wrong size");
    }
}

uint64_t BloomStore::blockCount() const {
    return (length + BlockSize - 1) / BlockSize;
}

void BloomStore::setBit(uint64_t bitPos) {
    (*backingFile.getData(bitPos / BlockSize)) |= BlockType{1} << (bitPos % BlockSize);
}

bool BloomStore::isSet(uint64_t bitPos) const {
    return !(((*backingFile.getData(bitPos / BlockSize)) & (BlockType{1} << (bitPos % BlockSize))) == 0);
}

void BloomStore::reset(uint64_t length) {
    backingFile.truncate(0);
    backingFile.truncate((length + BlockSize - 1) / BlockSize);
}

BloomFilterData::BloomFilterData() : maxItems(0), fpRate(1), m_numHashes(0), length(0), addedCount(0) {}
BloomFilterData::BloomFilterData(uint64_t maxItems_, double fpRate_) : maxItems(maxItems_), fpRate(fpRate_), m_numHashes(static_cast<uint8_t>(std::ceil(-std::log(fpRate) / Log2))), length(static_cast<uint64_t>(std::floor((-std::log(fpRate) * maxItems) / Log2Squared))), addedCount(0) {}


BloomFilterData loadData(const boost::filesystem::path &path, uint64_t maxItems, double fpRate) {
    BloomFilterData data{maxItems, fpRate};
    boost::filesystem::ifstream file(path, std::ios::binary);
    if (file.good()) {
        boost::archive::binary_iarchive ia(file);
        ia >> data;
    }
    return data;
}

BloomFilter::BloomFilter(const boost::filesystem::path &path_, uint64_t maxItems, double fpRate) : path(path_), impData(loadData(metaPath(), maxItems, fpRate)), store(storePath(), impData.length) {}

BloomFilter::~BloomFilter() {
    boost::filesystem::ofstream file(metaPath(), std::ios::binary);
    boost::archive::binary_oarchive oa(file);
    oa << impData;
}

void BloomFilter::reset(uint64_t maxItems, double fpRate) {
    impData = BloomFilterData(maxItems, fpRate);
    store.reset(impData.length);
}

inline std::array<uint64_t, 2> hash(const uint8_t *data, int len) {
    std::array<uint64_t, 2> hashValue;
    MurmurHash3_x64_128(data, len, 0, hashValue.data());
    
    return hashValue;
}

inline uint64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
    return (hashA + n * hashB) % filterSize;
}

void BloomFilter::add(const uint8_t *item, int length) {
    auto hashValues = hash(item, length);
    
    for (uint8_t n = 0; n < impData.m_numHashes; n++) {
        auto bitPos = nthHash(n, hashValues[0], hashValues[1], impData.length);
        store.setBit(bitPos);
    }
    
    impData.addedCount++;
}

bool BloomFilter::possiblyContains(const uint8_t *item, int length) const {
    auto hashValues = hash(item, length);
    
    for (uint8_t n = 0; n < impData.m_numHashes; n++) {
        auto bitPos = nthHash(n, hashValues[0], hashValues[1], impData.length);
        if (!store.isSet(bitPos)) {
            return false;
        }
    }
    
    return true;
}
