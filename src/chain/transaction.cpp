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
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/scripts/nulldata_script.hpp>

#include <internal/bitcoin_uint256_hex.hpp>
#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>
#include <internal/hash_index.hpp>
#include <internal/mempool_index.hpp>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip_with.hpp>

#include <sstream>

namespace {
    uint32_t getTxIndex(const blocksci::uint256 &hash, blocksci::HashIndex &index) {
        auto txIndex = index.getTxIndex(hash);
        if (txIndex) {
            return *txIndex;
        } else {
            throw blocksci::InvalidHashException();
        }
    }
}

namespace blocksci {

    Transaction::Transaction(uint32_t index, BlockHeight height, DataAccess &access_) :
    Transaction(access_.getChain().getTxData(index), index, height, static_cast<uint32_t>(access_.getChain().txCount()), access_) {}
    
    Transaction::Transaction(uint32_t index, DataAccess &access_) : Transaction(index, access_.getChain().getBlockHeight(index), access_) {}
    
    Transaction::Transaction(const uint256 &hash, DataAccess &access_) : Transaction(getTxIndex(hash, access_.getHashIndex()), access_) {}

    Transaction::Transaction(const std::string &hash, DataAccess &access_) : Transaction(uint256S(hash), access_) {}
    
    std::string Transaction::toString() const {
        std::stringstream ss;
        ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << getBlockHeight() <<", tx_index=" << txNum << ")";
        return ss.str();
    }
    
    std::ostream &operator<<(std::ostream &os, const Transaction &tx) {
        return os << tx.toString();
    }
    
    bool isSegwitMarker(const Transaction &tx) {
        for (uint16_t i = 0; i < tx.outputCount(); i++) {
            auto output = tx.outputs()[tx.outputCount() - 1 - i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto nulldata = script::OpReturn(output.getAddress().scriptNum, tx.getAccess());
                if (nulldata.isSegwitMarker()) {
                    return true;
                }
            }
        }
        return false;
    }
    
    BlockHeight Transaction::calculateBlockHeight() const {
        return access->getChain().getBlockHeight(txNum);
    }
    
    ranges::optional<std::chrono::system_clock::time_point> Transaction::getTimeSeen() const {
        return access->getMempoolIndex().getTxTime(txNum);
    }

    ranges::optional<uint32_t> Transaction::getTimestampSeen() const {
        auto ts = access->getMempoolIndex().getTxTimestamp(txNum);
        if (ts) {
            return static_cast<uint32_t>(ts.value());
        } else {
            return ranges::nullopt;
        }
    }
    
    bool Transaction::observedInMempool() const {
        return access->getMempoolIndex().observed(txNum);
    }
    
    Block Transaction::block() const {
        return {getBlockHeight(), *access};
    }
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer) const {
        std::vector<OutputPointer> pointers;
        auto input = Input(pointer, *access);
        auto address = input.getAddress();
        auto search = Inout{pointer.txNum, address.scriptNum, address.type, input.getValue()};
        uint16_t i = 0;
        RANGES_FOR (auto output, outputs()) {
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
        RANGES_FOR (auto input, inputs()) {
            if (input == search) {
                pointers.emplace_back(txNum, i);
            }
            i++;
        }
        return pointers;
    }
    
    ranges::optional<Output> getOpReturn(const Transaction &tx) {
        RANGES_FOR (auto output, tx.outputs()) {
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                return output;
            }
        }
        return ranges::nullopt;
    }
    
    bool includesOutputOfType(const Transaction &tx, AddressType::Enum type) {
        return ranges::any_of(tx.outputs(), [=](const Output &output) {
            return output.getType() == type;
        });
    }
    
    bool hasFeeGreaterThan(Transaction &tx, int64_t txFee) {
        return fee(tx) > txFee;
    }
} // namespace blocksci
