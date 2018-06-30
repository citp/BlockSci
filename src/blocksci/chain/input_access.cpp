//
//  input_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "input.hpp"
#include "data_access.hpp"
#include "transaction.hpp"

namespace blocksci {
    Transaction Input::getSpentTx() const {
        return Transaction::txWithIndex(DataAccess::Instance().chain, linkedTxNum);
    }
}
