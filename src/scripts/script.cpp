//
//  script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#include <blocksci/scripts/script.hpp>
#include <blocksci/chain/transaction.hpp>

namespace blocksci {
    Transaction ScriptBase::getFirstTransaction() const {
        return Transaction(getFirstTxIndex(), getAccess());
    }
    
    ranges::optional<Transaction> ScriptBase::getTransactionRevealed() const {
        auto index = getTxRevealedIndex();
        if (index) {
            return Transaction(*index, getAccess());
        } else {
            return ranges::nullopt;
        }
    }
} // namespace blocksci
