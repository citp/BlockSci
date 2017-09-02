//
//  utilities.c
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "basic_types.hpp"

#include <ostream>
#include <cstdio>

std::ostream &operator<<(std::ostream &os, RawOutputPointer const &pointer) {
    os << pointer.hash.GetHex();
    os << ":";
    os << pointer.outputNum;
    return os;
}
