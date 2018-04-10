//
//  transaction.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "transaction.hpp"

#include "algorithms.hpp"
#include "block.hpp"
#include "inout_pointer.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/scripts/script_variant.hpp>

namespace blocksci {
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
    
    bool hasFeeGreaterThan(Transaction &tx, uint64_t txFee) {
        return fee(tx) > txFee;
    }
} // namespace blocksci
