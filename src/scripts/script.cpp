//
//  script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/core/address_info.hpp>

namespace blocksci {
    
    ScriptBase::ScriptBase(const Address &address) : ScriptBase(address.scriptNum, address.type, address.getAccess(), address.getAccess().scripts->getScriptHeader(address.scriptNum, dedupType(address.type))) {}
    
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
