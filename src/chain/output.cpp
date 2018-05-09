//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/block.hpp>

namespace blocksci {
    Transaction Output::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    Block Output::block() const {
        return {getBlockHeight(), *access};
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
