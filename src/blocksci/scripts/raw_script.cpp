//
//  raw_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/28/17.
//

#include "raw_script.hpp"

#include "hash.hpp"

namespace std {
    size_t hash<blocksci::RawScript>::operator()(const blocksci::RawScript &b) const {
        std::size_t seed = 8957643;
        
        hash_combine(seed, b.hash);
        hash_combine(seed, b.type);
        return seed;
    };
}
