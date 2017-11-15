//
//  block.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "block.hpp"
#include "chain_access.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"
#include "transaction_summary.hpp"
#include "address/address.hpp"
#include "scripts/scripts.hpp"

#include <range/v3/view/transform.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/all.hpp>

#include <numeric>
#include <fstream>
#include <sstream>

namespace blocksci {
    
    extern template std::string base_blob<256>::GetHex() const;
    
    RawBlock::RawBlock(uint32_t firstTxIndex_, uint32_t numTxes_, uint32_t height_, uint256 hash_, int32_t version_, uint32_t timestamp_, uint32_t bits_, uint32_t nonce_, uint64_t coinbaseOffset_) : firstTxIndex(firstTxIndex_), numTxes(numTxes_), height(height_), hash(hash_), version(version_), timestamp(timestamp_), bits(bits_), nonce(nonce_), coinbaseOffset(coinbaseOffset_) {}
    
    bool RawBlock::operator==(const RawBlock& other) const {
        return firstTxIndex == other.firstTxIndex
        && numTxes == other.numTxes
        && height == other.height
        && hash == other.hash
        && version == other.version
        && timestamp == other.timestamp
        && bits == other.bits
        && nonce == other.nonce
        && coinbaseOffset == other.coinbaseOffset;
    }
    
    std::string RawBlock::getString() const {
        std::stringstream ss;
        ss << "Block(numTxes=" << numTxes <<", height=" << height <<", header_hash=" << hash.GetHex() << ", version=" << version <<", timestamp=" << timestamp << ", bits=" << bits << ", nonce=" << nonce << ")";
        return ss.str();
    }
    
    Block::Block(uint32_t blockNum, const ChainAccess &access_) : access(&access_), rawBlock(access->getBlock(blockNum)) {
        
    }
    
    bool Block::cursor::equal(cursor const &that) const {
        return currentTxIndex == that.currentTxIndex;
    }
    
    int Block::cursor::distance_to(cursor const &that) const {
        return static_cast<int>(that.currentTxIndex) - static_cast<int>(currentTxIndex);
    }
    
    void Block::cursor::next() {
        auto tx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        currentTxPos += sizeof(RawTransaction) +
        static_cast<size_t>(tx->inputCount) * sizeof(Input) +
        static_cast<size_t>(tx->outputCount) * sizeof(Output);
        currentTxIndex++;
    }
    
    void Block::cursor::prev() {
        currentTxIndex--;
        currentTxPos = nullptr;
    }
    
    void Block::cursor::advance(int amount) {
        currentTxIndex += static_cast<uint32_t>(amount);
        currentTxPos = nullptr;
    }
    
    Transaction Block::cursor::read() const {
        if (currentTxPos == nullptr) {
            currentTxPos = reinterpret_cast<const char *>(block->access->getTx(currentTxIndex));
        }
        auto rawTx = reinterpret_cast<const RawTransaction *>(currentTxPos);
        return {*block->access, rawTx, currentTxIndex, block->height()};
    }
    
    const std::string Block::getHeaderHash() const {
        return rawBlock->hash.GetHex();
    }
    
    std::vector<unsigned char> Block::getCoinbase() const {
        return access->getCoinbase(rawBlock->coinbaseOffset);
    }
    
    std::chrono::system_clock::time_point Block::getTime() const {
        return std::chrono::system_clock::from_time_t(static_cast<time_t>(rawBlock->timestamp));
    }
    
    Transaction Block::coinbaseTx() const {
        return (*this)[0];
    }
    
    bool isSegwit(const Block &block, const ScriptAccess &scripts) {
        auto coinbase = block.coinbaseTx();
        for (int i = coinbase.outputCount() - 1; i >= 0; i--) {
            auto &output = coinbase.outputs()[i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto nulldata = script::OpReturn(scripts, output.toAddressNum);
                uint32_t startVal = *reinterpret_cast<const uint32_t *>(nulldata.data.c_str());
                if (startVal == 0xaa21a9ed) {
                    return true;
                }
            }
        }
        return false;
    }
    
    /**
     * Compact Size
     * size <  253        -- 1 byte
     * size <= USHRT_MAX  -- 3 bytes  (253 + 2 bytes)
     * size <= UINT_MAX   -- 5 bytes  (254 + 4 bytes)
     * size >  UINT_MAX   -- 9 bytes  (255 + 8 bytes)
     */
    inline unsigned int GetSizeOfCompactSize(uint64_t nSize)
    {
        if (nSize < 253)             return sizeof(unsigned char);
        else if (nSize <= std::numeric_limits<unsigned short>::max()) return sizeof(unsigned char) + sizeof(unsigned short);
        else if (nSize <= std::numeric_limits<unsigned int>::max())  return sizeof(unsigned char) + sizeof(unsigned int);
        else                         return sizeof(unsigned char) + sizeof(uint64_t);
    }
    
    size_t sizeBytes(const Block &block) {
        size_t size = GetSizeOfCompactSize(block.size()) + 80;
        for (auto tx : block) {
            size += tx.sizeBytes();
        }
        return size;
    }
    
    std::vector<uint64_t> fees(const Block &block) {
        return block | ranges::view::transform(fee) | ranges::to_vector;
    }
    
    std::vector<double> feesPerByte(const Block &block) {
        return block | ranges::view::transform(feePerByte) | ranges::to_vector;
    }
    
    TransactionSummary transactionStatistics(const Block &block) {
        return ranges::accumulate(block, TransactionSummary{});
        
    }
    
    std::vector<const Output *> getUnspentOutputs(const Block &block, const ChainAccess &access) {
        std::vector<const Output *> outputs;
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                if (!output.isSpent(access)) {
                    outputs.push_back(&output);
                }
            }
        }
        return outputs;
    }
    
    std::vector<const Output *> getOutputsSpentByHeight(const Block &block, uint32_t height, const ChainAccess &access) {
        std::vector<const Output *> outputs;
        auto lastTxIndex = Block(height, access).endTxIndex();
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                auto spentIndex = output.getSpendingTxIndex(access);
                if (spentIndex > 0 && spentIndex < lastTxIndex) {
                    outputs.push_back(&output);
                }
            }
        }
        return outputs;
    }
    
    uint64_t totalOut(const Block &block) {
        uint64_t total = 0;
        for (auto tx : block) {
            total += totalOut(tx);
        }
        return total;
    }
    
    uint64_t totalIn(const Block &block) {
        uint64_t total = 0;
        for (auto tx : block) {
            total += totalIn(tx);
        }
        return total;
    }
    
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height, const ChainAccess &access) {
        uint64_t total = 0;
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                if (!output.isSpent(access) || output.getSpendingTx(access)->blockHeight > height) {
                    total += output.getValue();
                }
            }
        }
        return total;
    }
    
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block) {
        std::unordered_map<AddressType::Enum, int64_t> net;
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                net[output.getType()] += output.getValue();
            }
            for (auto &input : tx.inputs()) {
                net[input.getType()] -= input.getValue();
            }
        }
        return net;
    }

    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block, const ScriptAccess &scripts) {
        std::unordered_map<std::string, int64_t> net;
        for (auto tx : block) {
            for (auto &output : tx.outputs()) {
                net[output.getAddress().fullType(scripts)] += output.getValue();
            }
            for (auto &input : tx.inputs()) {
                net[input.getAddress().fullType(scripts)] -= input.getValue();
            }
        }
        return net;
    }
    
    uint64_t getTotalSpentOfAge(const Block &block, const ChainAccess &access, uint32_t age) {
        uint64_t total = 0;
        uint32_t newestTxNum = Block(block.height() - age, access).endTxIndex() - 1;
        for (auto tx : block) {
            for (auto &input : tx.inputs()) {
                if (input.spentTxIndex() <= newestTxNum) {
                    total += input.getValue();
                }
            }
        }
        return total;
    }
    
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, const ChainAccess &access, uint32_t maxAge) {
        std::vector<uint64_t> totals(maxAge);
        uint32_t newestTxNum = Block(block.height() - 1, access).endTxIndex() - 1;
        for (auto tx : block) {
            for (auto &input : tx.inputs()) {
                if (input.spentTxIndex() <= newestTxNum) {
                    uint32_t age = std::min(maxAge, block.height() - input.getSpentTx(access).block(access).height()) - 1;
                    totals[age] += input.getValue();
                }
            }
        }
        for (size_t i = 1; i < maxAge; i++) {
            totals[maxAge - i - 1] += totals[maxAge - i];
        }
        return totals;
    }
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &block) {
    os << block.getString();
    return os;
}

