//
//  script_variant_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/23/17.
//

#include "script_variant.hpp"
#include "util/data_access.hpp"
#include "chain/transaction.hpp"

namespace blocksci {
    AnyScript::AnyScript(const Address &address) : AnyScript(address, *DataAccess::Instance().scripts) {}
    AnyScript::AnyScript(const Script &script) : AnyScript(script, *DataAccess::Instance().scripts) {}
    
    Transaction AnyScript::getFirstTransaction() const {
        return getFirstTransaction(*DataAccess::Instance().chain);
    }
    
    ranges::optional<Transaction> AnyScript::getTransactionRevealed() const {
        return getTransactionRevealed(*DataAccess::Instance().chain);
    }
}
