//
//  raw_input.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "input.hpp"
#include "block.hpp"
#include "transaction.hpp"
#include "inout_pointer.hpp"
#include "chain_access.hpp"
#include "address/address.hpp"
#include "util/hash.hpp"
#include "scripts/script_variant.hpp"

#include <sstream>

namespace blocksci {
    
    Transaction Input::transaction() const {
        return Transaction(pointer.txNum, blockHeight, *access);
    }
    
    Address Input::getAddress() const {
        return Address(inout->toAddressNum, inout->getType(), *access);
    }
    
    Block Input::block() const {
        return Block(blockHeight, *access);
    }
    
    Transaction Input::getSpentTx() const {
        return Transaction(inout->linkedTxNum, *access);
    }
    
    std::string Input::toString() const {
        std::stringstream ss;
        ss << "TxIn(spent_tx_index=" << inout->linkedTxNum << ", address=" << getAddress().getScript().toString() <<", value=" << inout->getValue() << ")";
        return ss.str();
    }

    uint32_t Input::age() const {
        return blockHeight - getSpentTx().blockHeight;
    }
}

namespace std {
    size_t hash<blocksci::Input>::operator()(const blocksci::Input &input) const {
        std::size_t seed = 3458697;
        hash_combine(seed, *input.inout);
        return seed;
    }
}
