//
//  address_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "script.hpp"
#include "util/data_access.hpp"
#include "script_variant.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"

namespace blocksci {
    Transaction Script::getFirstTransaction() const {
        return getFirstTransaction(*DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> Script::getTransactionRevealed() const {
        return getTransactionRevealed(*DataAccess::Instance().chain);
    }
}
