//
//  output.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/address.hpp>

#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>

#include <sstream>

namespace blocksci {
    Output::Output(const OutputPointer &pointer_, DataAccess &access_) :
    Output(pointer_, -1, access_.getChain().getTx(pointer_.txNum)->getOutput(pointer_.inoutNum), static_cast<uint32_t>(access_.getChain().txCount()), access_) {}
    
    Transaction Output::transaction() const {
        return {pointer.txNum, blockHeight, *access};
    }
    
    Block Output::block() const {
        return {getBlockHeight(), *access};
    }
    
    Address Output::getAddress() const {
        return {inout->getAddressNum(), inout->getType(), *access};
    }
    
    ranges::optional<Transaction> Output::getSpendingTx() const {
        auto index = getSpendingTxIndex();
        if (index) {
            return ranges::optional<Transaction>{Transaction(*index, access->getChain().getBlockHeight(*index), *access)};
        } else {
            return ranges::nullopt;
        }
    }
    
    BlockHeight Output::getBlockHeight() const {
        if (blockHeight == -1) {
            blockHeight = access->getChain().getBlockHeight(pointer.txNum);
        }
        return blockHeight;
    }
    
    std::string Output::toString() const {
        std::stringstream ss;
        ss << "TxOut(spending_tx_index=" << inout->getLinkedTxNum() << ", address=" << getAddress().toString() << ", value=" << inout->getValue() << ")";
        return ss.str();
    }
    
    std::ostream &operator<<(std::ostream &os, const Output &output) {
        return os << output.toString();
    }
} // namespace blocksci
