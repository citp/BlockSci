//
//  utilities.c
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "basic_types.hpp"

#include <boost/functional/hash.hpp>

#include <ostream>

std::size_t hash_value(const RawOutputPointer &pointer) {
    std::size_t seed = 5764245;
    boost::hash_combine(seed, pointer.hash);
    boost::hash_combine(seed, pointer.outputNum);
    return seed;
}

std::ostream &operator<<(std::ostream &os, RawOutputPointer const &pointer) {
    os << pointer.hash.GetHex();
    os << ":";
    os << pointer.outputNum;
    return os;
}

