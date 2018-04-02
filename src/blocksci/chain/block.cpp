//
//  block.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "block.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"
#include "transaction_summary.hpp"
#include "address/address.hpp"
#include "scripts/nulldata_script.hpp"

#include <range/v3/to_container.hpp>
#include <range/v3/range_for.hpp>
#include <range/v3/view/remove_if.hpp>

#include <numeric>
#include <fstream>
#include <sstream>

namespace blocksci {
    
    std::string Block::getString() const {
        std::stringstream ss;
        ss << "Block(numTxes=" << rawBlock->numTxes <<", height=" << blockNum <<", header_hash=" << rawBlock->hash.GetHex() << ", version=" << rawBlock->version <<", timestamp=" << rawBlock->timestamp << ", bits=" << rawBlock->bits << ", nonce=" << rawBlock->nonce << ")";
        return ss.str();
    }

    Block Block::nextBlock() const {
        return {blockNum + BlockHeight(1), *access};
    }

    Block Block::prevBlock() const {
        return {blockNum - BlockHeight(1), *access};
    }
    
    const std::string Block::getHeaderHash() const {
        return rawBlock->hash.GetHex();
    }
    
    std::vector<unsigned char> Block::getCoinbase() const {
        return access->chain->getCoinbase(rawBlock->coinbaseOffset);
    }
    
    std::chrono::system_clock::time_point Block::getTime() const {
        return std::chrono::system_clock::from_time_t(static_cast<time_t>(rawBlock->timestamp));
    }
    
    Transaction Block::coinbaseTx() const {
        return (*this)[0];
    }
    
    bool isSegwit(const Block &block) {
        auto coinbase = block.coinbaseTx();
        for (int i = coinbase.outputCount() - 1; i >= 0; i--) {
            auto output = coinbase.outputs()[i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto nulldata = script::OpReturn(output.getAddress().scriptNum, block.getAccess());
                auto data = nulldata.getData();
                uint32_t startVal = *reinterpret_cast<const uint32_t *>(data.c_str());
                if (startVal == 0xaa21a9ed) {
                    return true;
                }
            }
        }
        return false;
    }
    
    TransactionSummary transactionStatistics(const Block &block) {
        return ranges::accumulate(block, TransactionSummary{});
    }
    
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block) {
        std::unordered_map<AddressType::Enum, int64_t> net;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                net[output.getType()] += output.getValue();
            }
            RANGES_FOR(auto input, tx.inputs()) {
                net[input.getType()] -= input.getValue();
            }
        }
        return net;
    }

    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block) {
        std::unordered_map<std::string, int64_t> net;
        RANGES_FOR(auto tx, block) {
            RANGES_FOR(auto output, tx.outputs()) {
                net[output.getAddress().fullType()] += output.getValue();
            }
            RANGES_FOR(auto input, tx.inputs()) {
                net[input.getAddress().fullType()] -= input.getValue();
            }
        }
        return net;
    }
    
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, BlockHeight maxAge) {
        std::vector<uint64_t> totals(static_cast<size_t>(static_cast<int>(maxAge)));
        uint32_t newestTxNum = block.prevBlock().endTxIndex() - 1;
        auto inputs = block.allInputs()
        | ranges::view::remove_if([=](const Input &input) { return input.spentTxIndex() > newestTxNum; });
        RANGES_FOR(auto input, inputs) {
            BlockHeight age = std::min(maxAge, block.height() - input.getSpentTx().block().height()) - BlockHeight{1};
            totals[static_cast<size_t>(static_cast<int>(age))] += input.getValue();
        }
        for (BlockHeight i{1}; i < maxAge; --i) {
            auto age = static_cast<size_t>(static_cast<int>(maxAge - i));
            totals[age - 1] += totals[age];
        }
        return totals;
    }
}

std::ostream &operator<<(std::ostream &os, blocksci::Block const &block) {
    os << block.getString();
    return os;
}

