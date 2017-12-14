//
//  transaction.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"
#include "block.hpp"
#include "chain_access.hpp"
#include "inout_pointer.hpp"
#include "algorithms.hpp"
#include "address/address.hpp"
#include "scripts/script_variant.hpp"
#include "index/hash_index.hpp"
#include "heuristics/change_address.hpp"

#include "util/hash.hpp"

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <sstream>

namespace blocksci {
    
    uint256 Transaction::getHash() const {
        return *access->getTxHash(txNum);
    }
    
    Block Transaction::block() const {
        return Block(blockHeight, *access);
    }
    
    std::string Transaction::toString() const {
        std::stringstream ss;
        ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << blockHeight <<", tx_index=" << txNum << ")";
        return ss.str();
    }
    
    
    uint32_t getTxIndex(uint256 hash, const HashIndex &index) {
        auto txIndex = index.getTxIndex(hash);
        if (txIndex == 0) {
            throw InvalidHashException();
        }
        return txIndex;
    }
    
    Transaction::Transaction(uint256 hash, const HashIndex &index, const ChainAccess &chain) : Transaction(getTxIndex(hash, index), chain) {}
    
    Transaction::Transaction(std::string hash, const HashIndex &index, const ChainAccess &chain) : Transaction(uint256S(hash), index, chain) {}
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer) const {
        std::vector<OutputPointer> pointers;
        auto input = Input(pointer, *access);
        auto search = Inout{pointer.txNum, input.getAddress(), input.getValue()};
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
        auto search = Inout{pointer.txNum, output.getAddress(), output.getValue()};
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
}
