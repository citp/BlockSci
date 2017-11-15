//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "output.hpp"
#include "input.hpp"
#include "chain_access.hpp"
#include "transaction.hpp"
#include "address/address.hpp"

#include <sstream>

namespace blocksci {
    
    Input Output::matchedInput(uint32_t txIndex) const {
        return {txIndex, getAddress(), getValue()};
    }
    
    std::string Output::toString() const {
        std::stringstream ss;
        auto address = getAddress();
        ss << "TxOut(tx_index_to=" << linkedTxNum << ", address=" << address <<", satoshis=" << getValue() << ")";
        return ss.str();
    }
    
    uint32_t Output::getSpendingTxIndex(const ChainAccess &access) const {
        if (linkedTxNum < access.maxLoadedTx()) {
            return linkedTxNum;
        } else {
            return 0;
        }
    }
    
    ranges::optional<Transaction> Output::getSpendingTx(const ChainAccess &access) const {
        if (isSpent(access)) {
            return Transaction::txWithIndex(access, getSpendingTxIndex(access));
        } else {
            return ranges::nullopt;
        }
    }
}

std::ostream &operator<<(std::ostream &os, blocksci::Output const &output) {
    os << output.toString();
    return os;
}
