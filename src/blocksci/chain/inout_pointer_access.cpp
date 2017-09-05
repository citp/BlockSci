//
//  inout_pointer_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "inout_pointer.hpp"
#include "data_access.hpp"
#include "transaction.hpp"

namespace blocksci {
    Transaction InoutPointer::getTransaction() const {
        return getTransaction(DataAccess::Instance().chain);
    }
}
