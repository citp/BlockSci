//
//  bloom_filter.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/29/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef bloom_filter_hpp
#define bloom_filter_hpp

#include <blocksci/util/file_mapper.hpp>

#include <boost/serialization/access.hpp>

#include <fstream>
#include <vector>

struct BloomStore {
    using BlockType = size_t;
    static constexpr size_t BlockSize = sizeof(BlockType) * 8;

    BloomStore(boost::filesystem::path path, uint64_t length);
    
    void setBit(uint64_t bitPos);
    bool isSet(uint64_t bitPos) const;
    
    void reset(uint64_t length);
    
private:
    std::array<BloomStore::BlockType, BloomStore::BlockSize> bitMasks;
    
private:
    blocksci::FixedSizeFileMapper<BlockType, blocksci::AccessMode::readwrite> backingFile;
    uint64_t length;
    
    uint64_t blockCount() const;
};

struct BloomFilterData {
    uint64_t maxItems;
    double fpRate;
    uint8_t m_numHashes;
    uint64_t length;
    uint64_t addedCount;
    
    BloomFilterData();
    BloomFilterData(uint64_t maxItems_, double fpRate_);
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & maxItems;
        ar & fpRate;
        ar & m_numHashes;
        ar & length;
        ar & addedCount;
    }
};

class BloomFilter {
public:
    // Load or create
    BloomFilter(boost::filesystem::path path, uint64_t maxItems, double fpRate);
    ~BloomFilter();
    
    void reset(uint64_t maxItems, double fpRate);
    
    template<class Key>
    void add(const Key &key) {
        int len = static_cast<int>(sizeof(Key));
        auto item = reinterpret_cast<const uint8_t *>(&key);
        add(item, len);
    }
    
    template<class Key>
    bool possiblyContains(const Key &key) const {
        auto len = static_cast<int>(sizeof(Key));
        auto item = reinterpret_cast<const uint8_t *>(&key);
        return possiblyContains(item, len);
    }
    
    bool isFull() const {
        return impData.addedCount >= impData.maxItems;
    }
    
    size_t size() { return impData.addedCount; }
    
    uint64_t getMaxItems() const {
        return impData.maxItems;
    }
    
    double getFPRate() const {
        return impData.fpRate;
    }
    
    boost::filesystem::path metaPath() const {
        return boost::filesystem::path(path).concat("Meta.dat");
    }
    
    boost::filesystem::path storePath() const {
        return boost::filesystem::path(path).concat("Store");
    }
    
private:
    boost::filesystem::path path;
    BloomFilterData impData;
    BloomStore store;
    
    void add(const uint8_t *item, int length);
    bool possiblyContains(const uint8_t *item, int length) const;
};

#endif /* bloom_filter_hpp */
