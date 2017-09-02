//
//  raw_input.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "input.hpp"
#include "chain_access.hpp"
#include "output.hpp"
#include "transaction.hpp"
#include "scripts/address_pointer.hpp"

#include <sstream>

namespace blocksci {
    
    Transaction Input::getSpentTx(const ChainAccess &access) const {
        return Transaction::txWithIndex(access, linkedTxNum);
    }
    
    std::string Input::toString() const {
        std::stringstream ss;
        auto address = getAddressPointer();
        ss << "TxIn(tx_index_to=" << linkedTxNum << ", address=" << address <<", satoshis=" << getValue() << ")";
        return ss.str();
    }
}



