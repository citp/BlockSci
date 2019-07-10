//
//  utilities.h
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#ifndef bascic_types_h
#define bascic_types_h

#include <blocksci/core/bitcoin_uint256.hpp>

#include <functional>
#include <cstdint>

/** Represents a pointer to an output by tx hash and output number */
struct RawOutputPointer {
    blocksci::uint256 hash;
    uint16_t outputNum;
    
    bool operator==(const RawOutputPointer& other) const {
        return hash == other.hash && outputNum == other.outputNum;
    }
    
    RawOutputPointer() {}
    RawOutputPointer(const blocksci::uint256 &hash_, uint16_t outputNum_) : hash(hash_), outputNum(outputNum_) {}
};

namespace std {
    template<> struct hash<RawOutputPointer> {
        size_t operator()(const RawOutputPointer &pointer) const;
    };
}

std::ostream &operator<<(std::ostream &os, RawOutputPointer const &pointer);

#endif /* utilities_h */
