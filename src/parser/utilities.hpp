//
//  utilities.h
//  BlockParser2
//
//  Created by Harry Kalodner on 1/12/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#ifndef utilities_h
#define utilities_h

#include <cstdint>
#include <cstring>
#include <stdio.h>

uint32_t readVariableLengthInteger(const char **buffer);

template <typename Type>
Type readNext(const char **buffer)
{
    Type ret;
    memcpy(&ret, *buffer, sizeof(Type));
    *buffer+=sizeof(Type);
    return ret;
}

template <typename Type>
Type readNext(const unsigned char **buffer)
{
    Type ret = *reinterpret_cast<const Type *>(*buffer);
    *buffer+=sizeof(Type);
    return ret;
}

#endif /* utilities_h */
