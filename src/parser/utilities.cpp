//
//  utilities.c
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#include "utilities.hpp"

#include <assert.h>
#include <cstdio>


// reads a variable length integer.
// See the documentation from here:  https://en.bitcoin.it/wiki/Protocol_specification#Variable_length_integer
uint32_t readVariableLengthInteger(const char **buffer)
{
    uint32_t ret = 0;
    
    uint8_t v = readNext<uint8_t>(buffer);
    
    if ( v < 0xFD ) { // If it's less than 0xFD use this value as the unsigned integer
        ret = static_cast<uint32_t>(v);
    } else if (v == 0xFD) {
        ret = static_cast<uint32_t>(readNext<uint16_t>(buffer));
    } else if (v == 0xFE) {
        ret = readNext<uint32_t>(buffer);
    } else {
        ret = static_cast<uint32_t>(readNext<uint64_t>(buffer));
    }
    return ret;
}
