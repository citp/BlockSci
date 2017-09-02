//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "output.hpp"
#include "chain_access.hpp"
#include "transaction.hpp"
#include "scripts/address.hpp"

#include <sstream>

namespace blocksci {
    
    std::string Output::toString() const {
        std::stringstream ss;
        auto address = getAddressPointer();
        ss << "TxOut(tx_index_to=" << linkedTxNum << ", address=" << address <<", satoshis=" << getValue() << ")";
        return ss.str();
    }
    
    AddressPointer Output::getAddressPointer() const {
        return AddressPointer(toAddressNum, getType());
    }
    
    bool Output::operator==(const Output& otherOutput) const {
        return this == &otherOutput;
    }
    
    uint32_t Output::getSpendingTxIndex(const ChainAccess &access) const {
        if (linkedTxNum < access.maxLoadedTx()) {
            return linkedTxNum;
        } else {
            return 0;
        }
    }
    
    boost::optional<Transaction> Output::getSpendingTx(const ChainAccess &access) const {
        if (isSpent(access)) {
            return Transaction::txWithIndex(access, getSpendingTxIndex(access));
        } else {
            return boost::none;
        }
        
    }
}

std::ostream &operator<<(std::ostream &os, blocksci::Output const &output) {
    os << output.toString();
    return os;
}
