//
//  bloom_filter.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/29/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "bloom_filter.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <fstream>
#include <array>
#include <cmath>


constexpr double Log2 = 0.69314718056;
constexpr double Log2Squared = Log2 * Log2;

BloomStore::BloomStore(const std::string &path_, int64_t length_) : bitMasks{}, backingFile(path_), length(length_) {
    
    for (size_t i = 0; i < BloomStore::BlockSize; ++i) {
        bitMasks.at(i) = BloomStore::BlockType{1} << i;
    }
    
    if (backingFile.size() == 0) {
        backingFile.truncate(blockCount());
    }
    
    if (backingFile.size() != blockCount()) {
        throw std::runtime_error("Trying to open bloom filter of wrong size");
    }
}

int64_t BloomStore::blockCount() const {
    return (length + BlockSize - 1) / BlockSize;
}

void BloomStore::setBit(int64_t bitPos) {
    (*backingFile[bitPos / BlockSize]) |= bitMasks[bitPos % BlockSize];
}

bool BloomStore::isSet(int64_t bitPos) const {
    return !(((*backingFile[bitPos / BlockSize]) & bitMasks[bitPos % BlockSize]) == 0);
}

void BloomStore::reset(int64_t newLength) {
    backingFile.truncate(0);
    backingFile.truncate((newLength + BlockSize - 1) / BlockSize);
}

int64_t calculateLength(int64_t maxItems, double fpRate) {
    return static_cast<int64_t>(std::ceil(-(std::log(fpRate) * maxItems) / Log2Squared));
}

uint8_t calculateHashes(double fpRate) {
    return static_cast<uint8_t>(std::round(-std::log(fpRate) / Log2));
}

BloomFilterData::BloomFilterData() : maxItems(0), fpRate(1), m_numHashes(0), length(0), addedCount(0) {}
BloomFilterData::BloomFilterData(int64_t maxItems_, double fpRate_) : maxItems(maxItems_), fpRate(fpRate_), m_numHashes(calculateHashes(fpRate_)), length(calculateLength(maxItems_, fpRate_)), addedCount(0) {}


BloomFilterData loadData(const filesystem::path &path, int64_t maxItems, double fpRate) {
    BloomFilterData data{maxItems, fpRate};
    std::ifstream file(path.str(), std::ios::binary);
    if (file.good()) {
        boost::archive::binary_iarchive ia(file);
        ia >> data;
    }
    return data;
}

BloomFilter::BloomFilter(const std::string &path_, int64_t maxItems, double fpRate) : path(path_), impData(loadData(metaPath(), maxItems, fpRate)), store(storePath().str(), impData.length) {}

BloomFilter::~BloomFilter() {
    std::ofstream file(metaPath().str(), std::ios::binary);
    boost::archive::binary_oarchive oa(file);
    oa << impData;
}

void BloomFilter::reset(int64_t maxItems, double fpRate) {
    impData = BloomFilterData(maxItems, fpRate);
    store.reset(impData.length);
}

inline std::array<uint64_t, 2> hash(const uint8_t *data, int len) {
    uint64_t hashA, hashB;
    memcpy(&hashA, data + len - sizeof(uint64_t), sizeof(uint64_t));
    memcpy(&hashB, data + len - 2*sizeof(uint64_t), sizeof(uint64_t));
    return {{hashA, hashB}};
}

inline int64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, int64_t filterSize) {
    return static_cast<int64_t>((hashA + n * hashB) % static_cast<uint64_t>(filterSize));
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
