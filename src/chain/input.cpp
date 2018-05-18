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
#include <blocksci/address/address.hpp>

#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>

#include <sstream>

namespace blocksci {
    Input::Input(const InputPointer &pointer_, DataAccess &access_) :
    Input(pointer_, access_.getChain().getBlockHeight(pointer_.txNum), access_.getChain().getTx(pointer_.txNum)->getInput(pointer_.inoutNum), &access_.getChain().getSequenceNumbers(pointer_.txNum)[pointer_.inoutNum], access_) {}
    
    Transaction Input::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    BlockHeight Input::age() const {
        return blockHeight - access->getChain().getBlockHeight(inout->getLinkedTxNum());
    }
    
    Block Input::block() const {
        return {blockHeight, *access};
    }
    
    Address Input::getAddress() const {
        return {inout->getAddressNum(), inout->getType(), *access};
    }
    
    Transaction Input::getSpentTx() const {
        auto txNum = inout->getLinkedTxNum();
        return {txNum, access->getChain().getBlockHeight(txNum), *access};
    }
    
    std::string Input::toString() const {
        std::stringstream ss;
        ss << "TxIn(spent_tx_index=" << inout->getLinkedTxNum() << ", address=" << getAddress().toString() <<", value=" << inout->getValue() << ")";
        return ss.str();
    }
    
    std::ostream &operator<<(std::ostream &os, const Input &input) {
        return os << input.toString();
    }
} // namespace blocksci
