//
//  raw_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/28/17.
//

#ifndef raw_script_hpp
#define raw_script_hpp

#include <blocksci/scripts/script_type.hpp>
#include <blocksci/bitcoin_uint256.hpp>

#include <boost/functional/hash.hpp>

namespace blocksci {
    struct RawScript {
        uint160 hash;
        ScriptType::Enum type;
        
        bool operator==(const RawScript& other) const {
            return type == other.type && hash == other.hash;
        }
    };
}

namespace std {
    template <>
    struct hash<blocksci::RawScript> {
    public:
        size_t operator()(const blocksci::RawScript &b) const {
            std::size_t seed = 8957643;
            boost::hash_combine(seed, b.hash);
            boost::hash_combine(seed, b.type);
            return seed;
        }
    };
}

#endif /* raw_script_hpp */
