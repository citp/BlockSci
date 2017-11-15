//
//  output_pointer_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "output_pointer.hpp"
#include "data_access.hpp"
#include "transaction.hpp"
#include "input.hpp"

namespace blocksci {
    const Output &OutputPointer::getOutput() const {
        return getOutput(*DataAccess::Instance().chain);
    }
}
