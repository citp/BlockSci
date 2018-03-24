//
//  taint.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#include "taint.hpp"
#include <blocksci/chain/algorithms.hpp>

namespace blocksci { namespace heuristics {
    std::unordered_map<Output, uint64_t> getHaircutTainted(const Output &output, uint64_t taintedValue) {
        std::unordered_map<Output, uint64_t> outputs;
        auto spendingTx = output.getSpendingTx();
        if (spendingTx) {
            auto totalOut = static_cast<double>(totalOutputValue(*spendingTx));
            for (auto spendingOut : spendingTx->outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalOut;
                auto value = static_cast<uint64_t>(percentage * static_cast<double>(taintedValue));
                auto subTainted = getHaircutTainted(spendingOut, value);
                for (auto pair : subTainted) {
                    outputs[pair.first] += pair.second;
                }
            }
        } else {
            outputs[output] += taintedValue;
        }
        return outputs;
    }
}}
