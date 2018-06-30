//
//  output_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "output.hpp"
#include "data_access.hpp"
#include "transaction.hpp"

namespace blocksci {
    
    uint32_t Output::getSpendingTxIndex() const {
        return getSpendingTxIndex(DataAccess::Instance().chain);
    }
    
    boost::optional<Transaction> Output::getSpendingTx() const {
        return getSpendingTx(DataAccess::Instance().chain);
    }
}
