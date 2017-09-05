//
//  utilities.h
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#ifndef bascic_types_h
#define bascic_types_h

#include <blocksci/bitcoin_uint256.hpp>

#include <functional>

#include <cstdint>
#include <cstring>
#include <stdio.h>

struct RawOutputPointer {
    blocksci::uint256 hash;
    uint16_t outputNum;
    
    bool operator==(const RawOutputPointer& other) const {
        return outputNum == other.outputNum && hash == other.hash;
    }
    
    RawOutputPointer() {}
    RawOutputPointer(const blocksci::uint256 &hash_, uint16_t outputNum_) : hash(hash_), outputNum(outputNum_) {}
};

std::ostream &operator<<(std::ostream &os, RawOutputPointer const &pointer);

#endif /* utilities_h */
