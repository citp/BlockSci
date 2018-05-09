//
//  raw_input.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>

namespace blocksci {
    Transaction Input::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    Block Input::block() const {
        return {blockHeight, *access};
    }
    
    Transaction Input::getSpentTx() const {
        return {inout->getLinkedTxNum(), *access};
    }
} // namespace blocksci
