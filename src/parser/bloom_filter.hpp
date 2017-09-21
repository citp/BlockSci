//
//  bloom_filter.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 1/29/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef bloom_filter_hpp
#define bloom_filter_hpp

#include <fstream>
#include <vector>
#include <stdio.h>

template<class Key>
class BloomFilter {
public:
    using BlockType = size_t;
    static constexpr size_t BlockSize = sizeof(BlockType) * 8;
    
    BloomFilter();
    BloomFilter(uint64_t maxItems, double fpRate);
    
    void add(const Key &key);
    bool isFull() const {
        return addedCount >= maxItems;
    }
    
    bool possiblyContains(const Key &key) const;
    
    size_t size() { return addedCount; }
    
    friend std::ostream &operator<<(std::ostream &output, const BloomFilter &b ) {
        output.write(reinterpret_cast<const char*>(&b.maxItems), sizeof(b.maxItems));
        output.write(reinterpret_cast<const char*>(&b.fpRate), sizeof(b.fpRate));
        output.write(reinterpret_cast<const char*>(&b.m_numHashes), sizeof(b.m_numHashes));
        output.write(reinterpret_cast<const char*>(&b.length), sizeof(b.length));
        output.write(reinterpret_cast<const char*>(&b.addedCount), sizeof(b.addedCount));
        
        size_t sz = b.data.size();
        output.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        output.write(reinterpret_cast<const char*>(&b.data[0]), static_cast<long>(sz * sizeof(b.data[0])));
        return output;
    }
    
    friend std::istream &operator>>(std::istream &input, BloomFilter &b ) {
        input.read(reinterpret_cast<char*>(&b.maxItems), sizeof(b.maxItems));
        input.read(reinterpret_cast<char*>(&b.fpRate), sizeof(b.fpRate));
        input.read(reinterpret_cast<char*>(&b.m_numHashes), sizeof(b.m_numHashes));
        input.read(reinterpret_cast<char*>(&b.length), sizeof(b.length));
        input.read(reinterpret_cast<char*>(&b.addedCount), sizeof(b.addedCount));
        
        size_t sz;
        input.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        b.data.resize(sz);
        input.read(reinterpret_cast<char*>(&b.data[0]), static_cast<long>(sz * sizeof(b.data[0])));
        return input;
    }
    
    uint64_t getMaxItems() const {
        return maxItems;
    }
    
    double getFPRate() const {
        return fpRate;
    }
    
private:
    uint64_t maxItems;
    double fpRate;
    uint8_t m_numHashes;
    uint64_t length;
    uint64_t addedCount;
    std::vector<BlockType> data;
};



#endif /* bloom_filter_hpp */
