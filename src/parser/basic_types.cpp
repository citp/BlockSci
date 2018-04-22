//
//  utilities.c
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "basic_types.hpp"

#include <blocksci/core/hash_combine.hpp>

#include <ostream>

namespace std
{
    size_t hash<RawOutputPointer>::operator()(const RawOutputPointer &pointer) const {
        std::size_t seed = 844653;
        blocksci::hash_combine(seed, pointer.hash);
        blocksci::hash_combine(seed, pointer.outputNum);
        return seed;
    }
}


std::ostream &operator<<(std::ostream &os, RawOutputPointer const &pointer) {
    os << pointer.hash.GetHex();
    os << ":";
    os << pointer.outputNum;
    return os;
}

