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
#include <range/v3/range_for.hpp>

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

    /** Get the Input that spends this Output, if it was spent yet */
    ranges::optional<Input> Output::getSpendingInput() const {
        auto spendingTx = getSpendingTx();
        if (spendingTx) {
            RANGES_FOR(auto input, spendingTx->inputs()) {
                if (input.getSpentOutputPointer() == pointer) {
                    return input;
                }
            }
            throw std::runtime_error("Whoopsie. Something went terribly wrong");
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<InputPointer> Output::getSpendingInputPointer() const {
        auto index = getSpendingTxIndex();
        if (index) {
            auto rawTx = access->getChain().getTx(*index);
            auto spentOutNums = access->getChain().getSpentOutputNumbers(*index);
            for (uint16_t i = 0; i < rawTx->inputCount; i++) {
                const auto &input = rawTx->getInput(i);
                auto spentOutNum = spentOutNums[i];
                if (OutputPointer{input.getLinkedTxNum(), spentOutNum} == pointer) {
                    return InputPointer{*index, i};
                }
            }
            throw std::runtime_error("Whoopsie. Something went terribly wrong");
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
