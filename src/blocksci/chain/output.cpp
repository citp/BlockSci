//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "output.hpp"

#include "block.hpp"
#include "transaction.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <sstream>

namespace blocksci {
    Transaction Output::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    Address Output::getAddress() const {
        return {inout->getAddressNum(), inout->getType(), *access};
    }
    
    Block Output::block() const {
        return {blockHeight, *access};
    }
    
    std::string Output::toString() const {
        std::stringstream ss;
        ss << "TxOut(spending_tx_index=" << inout->getLinkedTxNum() << ", address=" << getAddress().getScript().toString() << ", value=" << inout->getValue() << ")";
        return ss.str();
    }
    
    ranges::optional<Transaction> Output::getSpendingTx() const {
        auto index = getSpendingTxIndex();
        if (index) {
            return ranges::optional<Transaction>{Transaction(*index, *access)};
        } else {
            return ranges::nullopt;
        }
    }
} // namespace blocksci
