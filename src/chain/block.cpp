//
//  block.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//  Copyright © 2018 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/block.hpp>

#include <range/v3/view/remove_if.hpp>

namespace blocksci {
    std::vector<int64_t> getTotalSpentOfAges(const Block &block, BlockHeight maxAge) {
        std::vector<int64_t> totals(static_cast<size_t>(static_cast<int>(maxAge)));
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
} // namespace blocksci
