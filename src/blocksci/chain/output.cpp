//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "output.hpp"
#include "inout_pointer.hpp"
#include "chain_access.hpp"
#include "transaction.hpp"
#include "util/hash.hpp"

#include <sstream>

namespace blocksci {
    
    Output::Output(const OutputPointer &pointer, const ChainAccess &access_) : Output(access_.getTx(pointer.txNum)->getOutput(pointer.inoutNum), access_) {}
    
    std::string Output::toString() const {
        std::stringstream ss;
        ss << "TxOut(tx_index_to=" << inout->linkedTxNum << ", address=" << inout->getAddress() <<", satoshis=" << inout->getValue() << ")";
        return ss.str();
    }
    
    bool Output::operator==(const Output &other) const {
        return *inout == *other.inout;
    }
    
    uint32_t Output::getSpendingTxIndex() const {
        if (inout->linkedTxNum < access->maxLoadedTx()) {
            return inout->linkedTxNum;
        } else {
            return 0;
        }
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

std::ostream &operator<<(std::ostream &os, blocksci::Output const &output) {
    os << output.toString();
    return os;
}
