//
//  raw_input.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "input.hpp"
#include "inout.hpp"
#include "transaction.hpp"
#include "inout_pointer.hpp"
#include "chain_access.hpp"
#include "address/address.hpp"
#include "hash.hpp"

#include <sstream>

namespace blocksci {
    
    Input::Input(const InputPointer &pointer, const ChainAccess &access_) : Input(access_.getTx(pointer.txNum)->getInput(pointer.inoutNum), access_) {}
    
    Transaction Input::getSpentTx() const {
        return Transaction(inout->linkedTxNum, *access);
    }
    
    std::string Input::toString() const {
        std::stringstream ss;
        ss << "TxIn(tx_index_to=" << inout->linkedTxNum << ", address=" << inout->getAddress() <<", satoshis=" << inout->getValue() << ")";
        return ss.str();
    }
}

namespace std {
    size_t hash<blocksci::Input>::operator()(const blocksci::Input &input) const {
        std::size_t seed = 3458697;
        hash_combine(seed, *input.inout);
        return seed;
    }
}
