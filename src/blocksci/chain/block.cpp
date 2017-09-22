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
#include "transaction_iterator.hpp"
#include "address/address.hpp"
#include "scripts/nulldata_script.hpp"

#include <boost/range/algorithm/copy.hpp>

#include <numeric>
#include <fstream>
#include <sstream>

namespace blocksci {
    
    extern template std::string base_blob<256>::GetHex() const;
    
    Block::Block(uint32_t firstTxIndex_, uint32_t numTxes_, uint32_t height_, uint256 hash_, int32_t version_, uint32_t timestamp_, uint32_t bits_, uint32_t nonce_, uint64_t coinbaseOffset_) : firstTxIndex(firstTxIndex_), numTxes(numTxes_), height(height_), hash(hash_), version(version_), timestamp(timestamp_), bits(bits_), nonce(nonce_), coinbaseOffset(coinbaseOffset_) {}
    
    bool Block::operator==(const Block& other) const {
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
    
    std::string Block::getString() const {
        std::stringstream ss;
        ss << "Block(numTxes=" << numTxes <<", height=" << height <<", header_hash=" << hash.GetHex() << ", version=" << version <<", timestamp=" << timestamp << ", bits=" << bits << ", nonce=" << nonce << ")";
        return ss.str();
    }
    
    const std::string Block::getHeaderHash() const {
        return hash.GetHex();
    }
    
    const Block &Block::create(const ChainAccess &access, uint32_t height) {
        return access.getBlock(height);
    }
    
    std::vector<unsigned char> Block::getCoinbase(const ChainAccess &access) const {
        return access.getCoinbase(coinbaseOffset);
    }
    
    std::chrono::system_clock::time_point Block::getTime() const {
        return std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp));
    }
    
    boost::iterator_range<TransactionIterator> Block::txes(const ChainAccess &access) const {
        return boost::make_iterator_range(begin(access), end(access));
    }
    
    Block::const_iterator Block::begin(const ChainAccess &access) const {
        return TransactionIterator(&access, firstTxIndex, height);
    }
    
    Block::const_iterator Block::end(const ChainAccess &access) const {
        return TransactionIterator(&access, firstTxIndex + numTxes, height);
    }
    
    Block::value_type Block::getTx(const ChainAccess &access, uint32_t txNum) const {
        auto num = firstTxIndex + txNum;
        return Transaction(access.getTx(num), num, height);
    }
    
    Transaction Block::coinbaseTx(const ChainAccess &access) const {
        return getTx(access, 0);
    }
    
    bool isSegwit(const Block &block, const ChainAccess &access, const ScriptAccess &scripts) {
        auto coinbase = block.coinbaseTx(access);
        for (int i = coinbase.outputCount() - 1; i >= 0; i--) {
            auto &output = coinbase.outputs()[i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto rawScript = output.getAddress().getScript(scripts);
                auto nulldata = dynamic_cast<script::OpReturn *>(rawScript.get());
                auto data = nulldata->data;
                uint32_t startVal = *reinterpret_cast<const uint32_t *>(data.c_str());
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
    
    size_t sizeBytes(const Block &block, const ChainAccess &access) {
        size_t size = GetSizeOfCompactSize(block.numTxes) + 80;
        for (auto tx : block.txes(access)) {
            size += tx.sizeBytes();
        }
        return size;
    }
    
    std::vector<uint64_t> fees(const Block &block, const ChainAccess &access) {
        std::vector<uint64_t> fees;
        fees.reserve(block.numTxes);
        boost::copy(block.txes(access) | boost::adaptors::transformed(fee), std::back_inserter(fees));
        return fees;
    }
    
    std::vector<double> feesPerByte(const Block &block, const ChainAccess &access) {
        std::vector<double> feesPerBytes;
        feesPerBytes.reserve(block.numTxes);
        boost::copy(block.txes(access) | boost::adaptors::transformed(feePerByte), std::back_inserter(feesPerBytes));
        return feesPerBytes;
    }
    
    TransactionSummary transactionStatistics(const Block &block, const ChainAccess &access) {
        return std::accumulate(block.begin(access), block.end(access), TransactionSummary{});
        
    }
    
    std::vector<const Output *> getUnspentOutputs(const Block &block, const ChainAccess &access) {
        std::vector<const Output *> outputs;
        for (auto tx : block.txes(access)) {
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
        auto blocks = access.getBlocks();
        uint32_t lastTxIndex = 0;
        if (height < blocks.size()) {
            lastTxIndex = blocks[height].getTx(access, 0).txNum;
        } else {
            lastTxIndex = blocks.back().getTx(access, 0).txNum;
        }
        for (auto tx : block.txes(access)) {
            for (auto &output : tx.outputs()) {
                auto spentIndex = output.getSpendingTxIndex(access);
                if (spentIndex > 0 && spentIndex < lastTxIndex) {
                    outputs.push_back(&output);
                }
            }
        }
        return outputs;
    }
    
    uint64_t totalOut(const Block &block, const ChainAccess &access) {
        uint64_t total = 0;
        for (auto tx : block.txes(access)) {
            total += totalOut(tx);
        }
        return total;
    }
    
    uint64_t totalIn(const Block &block, const ChainAccess &access) {
        uint64_t total = 0;
        for (auto tx : block.txes(access)) {
            total += totalIn(tx);
        }
        return total;
    }
    
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height, const ChainAccess &access) {
        uint64_t total = 0;
        for (auto tx : block.txes(access)) {
            for (auto &output : tx.outputs()) {
                if (!output.isSpent(access) || output.getSpendingTx(access)->blockHeight > height) {
                    total += output.getValue();
                }
            }
        }
        return total;
    }
    
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block, const ChainAccess &access) {
        std::unordered_map<AddressType::Enum, int64_t> net;
        for (auto tx : block.txes(access)) {
            for (auto &output : tx.outputs()) {
                net[output.getType()] += output.getValue();
            }
            for (auto &input : tx.inputs()) {
                net[input.getType()] -= input.getValue();
            }
        }
        return net;
    }
    
    uint64_t getTotalSpentOfAge(const Block &block, const ChainAccess &access, uint32_t age) {
        uint64_t total = 0;
        auto &oldBlock = access.getBlock(block.height - age);
        uint32_t newestTxNum = oldBlock.txes(access).back().txNum;
        for (auto tx : block.txes(access)) {
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
        uint32_t newestTxNum = access.getBlock(block.height - 1).txes(access).back().txNum;
        for (auto tx : block.txes(access)) {
            for (auto &input : tx.inputs()) {
                if (input.spentTxIndex() <= newestTxNum) {
                    uint32_t age = std::min(maxAge, block.height - access.getBlockHeight(input.spentTxIndex())) - 1;
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

