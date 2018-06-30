//
//  inout_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "inout_pointer.hpp"
#include "transaction.hpp"
#include "chain_access.hpp"

namespace blocksci {
    Transaction InoutPointer::getTransaction(const ChainAccess &access) const {
        return Transaction(access.getTx(txNum), txNum, access.getBlockHeight(txNum));
    }
}
