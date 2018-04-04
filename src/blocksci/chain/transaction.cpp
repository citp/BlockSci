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
#include "chain_access.hpp"
#include "inout_pointer.hpp"
#include "input.hpp"
#include "output.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/util/hash.hpp>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace blocksci {
    uint256 Transaction::getHash() const {
        return *access->chain->getTxHash(txNum);
    }
    
    Block Transaction::block() const {
        return {blockHeight, *access};
    }
    
    std::string Transaction::toString() const {
        std::stringstream ss;
        ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << blockHeight <<", tx_index=" << txNum << ")";
        return ss.str();
    }
    
    
    uint32_t getTxIndex(const uint256 &hash, HashIndex &index) {
        auto txIndex = index.getTxIndex(hash);
        if (txIndex == 0) {
            throw InvalidHashException();
        }
        return txIndex;
    }
    
    Transaction::Transaction(const uint256 &hash, const DataAccess &access) : Transaction(getTxIndex(hash, *access.hashIndex), access) {}
    
    Transaction::Transaction(const std::string &hash, const DataAccess &access) : Transaction(uint256S(hash), access) {}
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer) const {
        std::vector<OutputPointer> pointers;
        auto input = Input(pointer, *access);
        auto search = Inout{pointer.txNum, RawAddress{input.getAddress()}, input.getValue()};
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
        auto search = Inout{pointer.txNum, RawAddress{output.getAddress()}, output.getValue()};
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
