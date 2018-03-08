//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "util/util.hpp"
#include "script.hpp"
#include "script_data.hpp"
#include "script_info.hpp"

#include "script_variant.hpp"

#include "address/address.hpp"
#include "address/address_info.hpp"
#include "index/address_index.hpp"

#include "chain/transaction.hpp"
#include "chain/output.hpp"

#include <iostream>


namespace blocksci {
    
    Script::Script(uint32_t scriptNum_, AddressType::Enum type_, const ScriptDataBase &data, const ScriptAccess &scripts_) : Address(scriptNum_, type_), access(&scripts_), firstTxIndex(data.txFirstSeen), txRevealed(data.txFirstSpent) {}
    
    Transaction Script::getFirstTransaction(const ChainAccess &chain) const {
        return Transaction(firstTxIndex, chain);
    }
    
    ranges::optional<Transaction> Script::getTransactionRevealed(const ChainAccess &chain) const {
        if (hasBeenSpent()) {
            return Transaction(txRevealed, chain);
        } else {
            return ranges::nullopt;
        }
    }
}
