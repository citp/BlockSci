//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "output.hpp"
#include "block.hpp"
#include "inout_pointer.hpp"
#include "transaction.hpp"
#include "address/address.hpp"
#include "scripts/script_variant.hpp"
#include "util/hash.hpp"

#include <sstream>

namespace blocksci {
    
    Transaction Output::transaction() const {
        return Transaction(pointer.txNum, blockHeight, *access);
    }
    
    Block Output::block() const {
        return Block(blockHeight, *access);
    }
    
    std::string Output::toString() const {
        std::stringstream ss;
        ss << "TxOut(spending_tx_index=" << inout->linkedTxNum << ", address=" << getAddress().getScript().toPrettyString() << ", value=" << inout->getValue() << ")";
        return ss.str();
    }
    
    ranges::optional<Transaction> Output::getSpendingTx() const {
        if (isSpent()) {
            return Transaction(getSpendingTxIndex(), *access);
        } else {
            return ranges::nullopt;
        }
    }
}

namespace std
{
    size_t hash<blocksci::Output>::operator()(const blocksci::Output &output) const {
        std::size_t seed = 819543;
        hash_combine(seed, *output.inout);
        return seed;
    }
}
