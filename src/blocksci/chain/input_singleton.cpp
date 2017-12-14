//
//  input_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/15/17.
//

#include "input.hpp"
#include "util/data_access.hpp"

namespace blocksci {
    Input::Input(const InputPointer &pointer) : Input(pointer, *DataAccess::Instance().chain) {}
}

