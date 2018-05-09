//
//  transaction.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/scripts/nulldata_script.hpp>

namespace {
    uint32_t getTxIndex(const blocksci::uint256 &hash, blocksci::HashIndex &index) {
        auto txIndex = index.getTxIndex(hash);
        if (txIndex == 0) {
            throw blocksci::InvalidHashException();
        }
        return txIndex;
    }
}

namespace blocksci {

    Transaction::Transaction(const uint256 &hash, DataAccess &access) : Transaction(getTxIndex(hash, access.getHashIndex()), access) {}

    bool isSegwitMarker(const Transaction &tx) {
        for (int i = tx.outputCount() - 1; i >= 0; i--) {
            auto output = tx.outputs()[i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto nulldata = script::OpReturn(output.getAddress().scriptNum, tx.getAccess());
                if (nulldata.isSegwitMarker()) {
                    return true;
                }
            }
        }
        return false;
    }
    
    
    Block Transaction::block() const {
        return {blockHeight, *access};
    }
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer) const {
        std::vector<OutputPointer> pointers;
        auto input = Input(pointer, *access);
        auto address = input.getAddress();
        auto search = Inout{pointer.txNum, address.scriptNum, address.type, input.getValue()};
        uint16_t i = 0;
        for (auto output : outputs()) {
            if (output == search) {
                pointers.emplace_back(txNum, i);
            }
            i++;
        }
        return pointers;
    }
    
    std::vector<InputPointer> Transaction::getInputPointers(const OutputPointer &pointer) const {
        std::vector<InputPointer> pointers;
        auto output = Output(pointer, *access);
        auto address = output.getAddress();
        auto search = Inout{pointer.txNum, address.scriptNum, address.type, output.getValue()};
        uint16_t i = 0;
        for (auto input : inputs()) {
            if (input == search) {
                pointers.emplace_back(txNum, i);
            }
            i++;
        }
        return pointers;
    }
    
    ranges::optional<Output> getOpReturn(const Transaction &tx) {
        for (auto output : tx.outputs()) {
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                return output;
            }
        }
        return ranges::nullopt;
    }
    
    bool hasFeeGreaterThan(Transaction &tx, int64_t txFee) {
        return fee(tx) > txFee;
    }
} // namespace blocksci
