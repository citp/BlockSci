//
//  output_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/15/17.
//

#include "output.hpp"
#include "data_access.hpp"

namespace blocksci {
    Output::Output(const OutputPointer &pointer) : Output(pointer, *DataAccess::Instance().chain) {}
}


