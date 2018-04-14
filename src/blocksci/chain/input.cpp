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

#include <blocksci/scripts/script_variant.hpp>

#include <sstream>

namespace blocksci {
    Transaction Input::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    Block Input::block() const {
        return {blockHeight, *access};
    }
    
    std::string Input::toString() const {
        std::stringstream ss;
        ss << "TxIn(spent_tx_index=" << inout->getLinkedTxNum() << ", address=" << getAddress().getScript().toString() <<", value=" << inout->getValue() << ")";
        return ss.str();
    }
    
    Transaction Input::getSpentTx() const {
        return {inout->getLinkedTxNum(), *access};
    }

    BlockHeight Input::age() const {
        return blockHeight - getSpentTx().blockHeight;
    }
} // namespace blocksci
