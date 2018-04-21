//
//  block.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction_summary.hpp>
#include <blocksci/scripts/nulldata_script.hpp>

namespace blocksci {
    bool isSegwit(const Block &block) {
        auto coinbase = block.coinbaseTx();
        for (int i = coinbase.outputCount() - 1; i >= 0; i--) {
            auto output = coinbase.outputs()[i];
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                auto nulldata = script::OpReturn(output.getAddress().scriptNum, block.getAccess());
                auto data = nulldata.getData();
                uint32_t startVal;
                std::memcpy(&startVal, data.c_str(), sizeof(startVal));
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
    
} // namespace blocksci
