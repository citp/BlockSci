//
//  block.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/address/address.hpp>

#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>
#include <internal/mempool_index.hpp>

#include <sstream>

namespace blocksci {
    Block::Block(const RawBlock *rawBlock_, BlockHeight blockNum_, DataAccess &access_) : Block(rawBlock_, Transaction(access_.getChain().getTxData(rawBlock_->firstTxIndex), rawBlock_->firstTxIndex, blockNum_, static_cast<uint32_t>(access_.getChain().txCount()), access_)) {}
    
    Block::Block(BlockHeight blockNum_, DataAccess &access_) : Block(access_.getChain().getBlock(blockNum_), blockNum_, access_) {}
    
    ranges::optional<std::chrono::system_clock::time_point> Block::getTimeSeen() const {
        return getAccess().getMempoolIndex().getBlockTimestamp(height());
    }
    
    std::vector<unsigned char> Block::getCoinbase() const {
        return getAccess().getChain().getCoinbase(rawBlock->coinbaseOffset);
    }
    
    std::string Block::getHeaderHash() const {
        return rawBlock->hash.GetHex();
    }
    
    std::string Block::toString() const {
        std::stringstream ss;
        ss << "Block(tx_count=" << rawBlock->txCount <<", height=" << height() <<", header_hash=" << getHeaderHash() << ", version=" << rawBlock->version <<", timestamp=" << rawBlock->timestamp << ", bits=" << rawBlock->bits << ", nonce=" << rawBlock->nonce << ")";
        return ss.str();
    }
    
    std::ostream &operator<<(std::ostream &os, const blocksci::Block &block) {
        os << block.toString();
        return os;
    }
    
    std::unordered_map<AddressType::Enum, int64_t> BLOCKSCI_EXPORT netAddressTypeValue(const Block &block) {
        std::unordered_map<AddressType::Enum, int64_t> net;
        for (auto tx : block) {
            for (auto output : tx.outputs()) {
                net[output.getType()] += output.getValue();
            }
            for (auto input : tx.inputs()) {
                net[input.getType()] -= input.getValue();
            }
        }
        return net;
    }
    
    std::unordered_map<std::string, int64_t> BLOCKSCI_EXPORT netFullTypeValue(const Block &block) {
        std::unordered_map<std::string, int64_t> net;
        for (auto tx : block) {
            for (auto output : tx.outputs()) {
                net[output.getAddress().fullType()] += output.getValue();
            }
            for (auto input : tx.inputs()) {
                net[input.getAddress().fullType()] -= input.getValue();
            }
        }
        return net;
    }
    
} // namespace blocksci
